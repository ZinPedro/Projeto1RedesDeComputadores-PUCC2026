#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>  
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <time.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define INTERVALO_TESTE 5

//estrutura para armazenar os dados do cliente (numero do cliente e status de conexão)
typedef struct {
    int client_fd;
    int conectado;
} DadosCliente;

//função para utilização da primeira thread, responsável por receber os dados do cliente
void *receber_dados(void *arg){

    DadosCliente *cliente = (DadosCliente *)arg;
    //buffer para armazenar os dados recebidos do cliente
    char buffer[BUFFER_SIZE];
    //variável para armazenar o número de bytes recebidos
    ssize_t bytes_recebidos;    

    //receber mensagem do cliente
    //configuração do recv (socket, buffer, tamanho do buffer, flags)
    while(1){
        bytes_recebidos = recv(cliente->client_fd, buffer, BUFFER_SIZE - 1, 0);

        if(bytes_recebidos < 0) {
            cliente->conectado = 0; //marca o cliente como desconectado
            perror("Erro ao receber mensagem");
            break;
        }

        if(bytes_recebidos == 0) {
            cliente->conectado = 0; //marca o cliente como desconectado
            printf("Cliente desconectado.\n");
            break;
        }
        //verifica o tamanho da mensagem recebida e adiciona o terminador de string
        buffer[bytes_recebidos] = '\0'; // Adiciona o terminador de string
        printf("Mensagem recebida do cliente: %s\n", buffer);
        
    }
    
    return NULL;
}

//função para utilização da segunda thread, responsável por enviar periodicamente mensagens para o cliente
void *enviar_periodicamente(void *args){

    DadosCliente *cliente = (DadosCliente *)args;

    char mensagem[128];

    while(cliente->conectado) {
        sleep(5); //verifica conexão a cada 5 segundos

        //saí quando o cliente estiver desconectado
        if(!cliente->conectado){
            break;
        }

        time_t agora = time(NULL);
        struct tm *horario = localtime(&agora);

        strftime(mensagem, sizeof(mensagem), "%d/%m/%Y %H:%M\n", horario);

        if(send(cliente->client_fd, mansagem, strlen(mensagem), 0) < 0){
            perror("Erro ao enviar mensagem periodica");
            cliente->conectado = 0; //marca o cliente como desconectado
            break;
        }
    }


    return NULL;
}

int main(){

    int server_fd;
    int client_fd;

    //vetor para armazenar a hora e mensagem de conexao (11:23: CONECTADO!!)
    char mensagem[64];

    //variável para armazenar as threads de recebimento e envio periodico
    pthread_t thread_recebimento;
    pthread_t thread_envio_periodico;

    //informações do servidor e do cliente
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr); //tamanho do endereço do cliente

    //Criação do socket (IPv4, TCP, protocolo padrão)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Erro ao criar o socket");
        return 1;
    }

    printf("Socket criado com sucesso.\n");

    // Configuração do socket para reutilizar o endereço para evitar o erro "Address already in use" ao reiniciar o servidor rapidamente

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erro no setsockopt");
        close(server_fd);
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //configuração da bind (socket, endereço, tamanho do endereço)

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erro ao fazer bind");
        close(server_fd);
        return 1;
    }

    printf("Bind realizado com sucesso na porta %d.\n", PORT);

    //configuração do listen (socket, tamanho da fila de conexões)
    if(listen(server_fd, 1) < 0){
        perror("Erro no listen");
        close(server_fd);
        return 1;
    }

    //loop para aceitar multiplas conexões de clientes, uma por vez
    while(1){

        //resetar o tamanho do endereço do cliente antes de cada accept
        client_addr_len = sizeof(client_addr);

        printf("Servidor aguardando conexoes na porta %d...\n", PORT);

        //configuração do accept (socket, endereço do cliente, tamanho do endereço do cliente)
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_fd < 0) {
            perror("Erro no accept");
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }   

        printf("Conexao aceita de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        //registro do cliente em uma estrutura de dados para controle de conexão
        DadosCliente cliente;
        cliente.client_fd = client_fd;
        cliente.conectado = 1; //marca o cliente como conectado

        //obter a hora atual
        time_t agora = time(NULL);
        struct tm *horario = localtime(&agora);

        //Montar mensagem
        strftime(mensagem, sizeof(mensagem), "%H:%M: CONECTADO!!\n", horario);

        //enviar mensagem para o cliente
        printf("Enviando ao Cliente: %s", mensagem);
        //configuração do send (socket, mensagem, tamanho da mensagem, flags)
        if(send(client_fd, mensagem, strlen(mensagem), 0) < 0){
            perror("Erro ao enviar mensagem");
            close(client_fd);
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }

        int resultado_thread = pthread_create(&thread_recebimento, NULL, receber_dados, &cliente);
        int resultado_thread_envio = pthread_create(&thread_envio_periodico, NULL, enviar_periodicamente, &cliente);

        //verifica se a thread foi criada com sucesso
        if(resultado_thread != 0){
            //erro ao criar a thread, pthread_create já retorna um código de erro, que pode ser convertido em uma mensagem de erro usando strerror
            fprintf(stderr, "Erro ao criar thread de recebimento: %s\n", strerror(resultado_thread));
            close(client_fd);
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }

        
        pthread_join(thread_recebimento, NULL); //espera a thread de recebimento terminar
        pthread_join(thread_envio_periodico, NULL); //espera a thread de envio periodico terminar
        

        close(client_fd);
    }

    //fechar o socket do servidor, saida do loop ainda nao implementada
    close(server_fd);

    return 0;
}