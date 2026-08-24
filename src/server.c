#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//inclui o arquivo de cabeçalho da plataforma para compatibilidade entre Windows e Linux
#include "../include/platform.h"

#include "../include/protocol.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define INTERVALO_TESTE 60

//estrutura para armazenar os dados do cliente (numero do cliente e status de conexão)
typedef struct {
    socket_t client_fd;
    int conectado;
    mutex_t mutex;
    shared_data_t protocolo; // aqui é a variavel que vai ter acesso a mem compartilhada para gravar o que o usuario digitou
} DadosCliente;

//função para definir o status de conexão do cliente de forma thread-safe
void definir_conectado(DadosCliente *cliente, int valor){
    bloquear_mutex(&cliente->mutex);
    cliente->conectado = valor;
    liberar_mutex(&cliente->mutex);
}

//função para verificar o status de conexão do cliente de forma thread-safe
int verificar_conectado(DadosCliente *cliente){
    int conectado;
    bloquear_mutex(&cliente->mutex);
    conectado = cliente->conectado;
    liberar_mutex(&cliente->mutex);
    return conectado;
}

//função para utilização da primeira thread, responsável por receber os dados do cliente
THREAD_FUNC(receber_dados) {

    DadosCliente *cliente = (DadosCliente *)arg;
    //buffer para armazenar os dados recebidos do cliente
    char buffer[BUFFER_SIZE];
    //variável para armazenar o número de bytes recebidos
    socket_io_t bytes_recebidos;    

    //receber mensagem do cliente
    //configuração do recv (socket, buffer, tamanho do buffer, flags)
    while(1){
        bytes_recebidos = recv(cliente->client_fd, buffer, BUFFER_SIZE - 1, 0);

        if(bytes_recebidos == PLATFORM_SOCKET_ERRO) {
            definir_conectado(cliente, 0); //marca o cliente como desconectado
            mostrar_erro_socket("Erro ao receber mensagem");
            break;
        }

        if(bytes_recebidos == 0) {
            definir_conectado(cliente, 0); //marca o cliente como desconectado
            printf("Cliente desconectado.\n");
            break;
        }

        //verifica o tamanho da mensagem recebida e adiciona o terminador de string
        buffer[bytes_recebidos] = '\0'; // Adiciona o terminador de string
        parse_input(buffer, &cliente->protocolo);
        
    }
    
    return THREAD_RETURN;
}

//função para utilização da segunda thread, responsável por enviar periodicamente mensagens para o cliente
THREAD_FUNC(enviar_periodicamente) {

    DadosCliente *cliente = (DadosCliente *)arg;

    char mensagem[128];

    while(verificar_conectado(cliente)){ 
        dormir_segundos(INTERVALO_TESTE); //verifica conexão a cada 5 segundos

        //saí quando o cliente estiver desconectado
        if(!verificar_conectado(cliente)){
            break;
        }

    char resposta[BUFFER_SIZE];
    tipo_acao_t acao = process_shared_data(&cliente->protocolo, resposta, sizeof(resposta)); // processa qual acao foi executada

    if (acao == ACAO_DESCONECTAR)  // se foi desconectar, desconecta
    {
        definir_conectado(cliente, 0);
        desligar_socket(cliente->client_fd);
        break;
    }

    if (strlen(resposta) > 0) 
    {
        if (send(cliente->client_fd, resposta, (int)strlen(resposta), 0) == PLATFORM_SOCKET_ERRO) 
        {   
            mostrar_erro_socket("Erro ao enviar resposta do protocolo");
            definir_conectado(cliente, 0);
            break;
        }
    }

        time_t agora = time(NULL);
        struct tm horario;
        
        if(!obter_horario_local(&agora, &horario)) {
            fprintf(stderr, "Erro ao obter horário local.\n");
            continue;
        }

        strftime(mensagem, sizeof(mensagem), "%d/%m/%Y %H:%M\n", &horario);

        if(send(cliente->client_fd, mensagem, (int)strlen(mensagem), 0) == PLATFORM_SOCKET_ERRO){
            mostrar_erro_socket("Erro ao enviar mensagem periodica");
            definir_conectado(cliente, 0); //marca o cliente como desconectado
            desligar_socket(cliente->client_fd); //fecha o socket do cliente para interromper a thread de recebimento
            break;
        }
    }


    return THREAD_RETURN;
}

int main(){

    socket_t server_fd;
    socket_t client_fd;

    //vetor para armazenar a hora e mensagem de conexao (11:23: CONECTADO!!)
    char mensagem[64];

    //variável para armazenar as threads de recebimento e envio periodico
    thread_t thread_recebimento;
    thread_t thread_envio_periodico;

    //informações do servidor e do cliente
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};
    sock_len_t client_addr_len = sizeof(client_addr); //tamanho do endereço do cliente

    //inicializa o winsock no Windows
    if(!iniciar_sockets()) {
        return 1;
    }

    protocol_init(); // incializar o mutex

    //Criação do socket (IPv4, TCP, protocolo padrão)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == PLATFORM_SOCKET_INVALIDO) {
        mostrar_erro_socket("Erro ao criar o socket");
        finalizar_sockets(); //encerra o winsock que inicializamos no começo
        return 1;
    }

    printf("Socket criado com sucesso.\n");

    if(!configurar_reuso_endereco(server_fd)){
        mostrar_erro_socket("Erro ao configurar reuso de endereço (setsockopt)");

        fechar_socket(server_fd);
        finalizar_sockets(); //encerra o winsock que inicializamos no começo
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    //configuração da bind (socket, endereço, tamanho do endereço)

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == PLATFORM_SOCKET_ERRO) {
        mostrar_erro_socket("Erro ao fazer bind");
        fechar_socket(server_fd);
        finalizar_sockets();
        return 1;
    }

    printf("Bind realizado com sucesso na porta %d.\n", PORT);

    //configuração do listen (socket, tamanho da fila de conexões)
    if(listen(server_fd, 1) == PLATFORM_SOCKET_ERRO) {
        mostrar_erro_socket("Erro no listen");
        fechar_socket(server_fd);
        finalizar_sockets();
        return 1;
    }

    //loop para aceitar multiplas conexões de clientes, uma por vez
    while(1){

        //resetar o tamanho do endereço do cliente antes de cada accept
        client_addr_len = sizeof(client_addr);

        printf("Servidor aguardando conexoes na porta %d...\n", PORT);

        //configuração do accept (socket, endereço do cliente, tamanho do endereço do cliente)
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_fd == PLATFORM_SOCKET_INVALIDO) {
            mostrar_erro_socket("Erro no accept");
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }   

        printf("Conexao aceita de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        //registro do cliente em uma estrutura de dados para controle de conexão
        DadosCliente cliente;
        cliente.client_fd = client_fd;
        cliente.conectado = 1; //marca o cliente como conectado

        snprintf(cliente.protocolo.nome_usuario, MAX_NOME, "%s:%d", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));  // define o nome padrao do usuario como "IP:porta"
        cliente.protocolo.pendente = 0;
        cliente.protocolo.acao = ACAO_NENHUMA;


        //inicializa o mutex do cliente
        if(!iniciar_mutex(&cliente.mutex)) {
            fprintf(stderr, "Erro ao inicializar mutex para o cliente.\n");
            fechar_socket(client_fd);
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }

        //obter a hora atual
        time_t agora = time(NULL);
        struct tm horario;

        if(!obter_horario_local(&agora, &horario)){
            fprintf(stderr, "Erro ao obter horario local.\n");

            destruir_mutex(&cliente.mutex);
            fechar_socket(client_fd);

            continue;
        }

        //Montar mensagem
        strftime(mensagem, sizeof(mensagem), "%H:%M: CONECTADO!!\n", &horario);

        //enviar mensagem para o cliente
        printf("Enviando ao Cliente: %s", mensagem);
        //configuração do send (socket, mensagem, tamanho da mensagem, flags)
        if(send(client_fd, mensagem, (int)strlen(mensagem), 0) == PLATFORM_SOCKET_ERRO){
            mostrar_erro_socket("Erro ao enviar mensagem");
            destruir_mutex(&cliente.mutex); //destruir o mutex antes de sair
            fechar_socket(client_fd);
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }

        //criação da thread de recebimento
        if(!criar_thread(&thread_recebimento, receber_dados, &cliente)){
            fprintf(stderr, "Erro ao criar thread de recebimento.\n");
            destruir_mutex(&cliente.mutex); //destruir o mutex antes de sair
            fechar_socket(client_fd);
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }

        //criação da thread de envio periodico
        if(!criar_thread(&thread_envio_periodico, enviar_periodicamente, &cliente)){
            fprintf(stderr, "Erro ao criar thread de envio periodico.\n");
            definir_conectado(&cliente, 0); //marca o cliente como desconectado
            desligar_socket(cliente.client_fd); //fecha o socket do cliente para interromper a thread de recebimento
            aguardar_thread(thread_recebimento); //espera a thread de recebimento terminar
            destruir_mutex(&cliente.mutex); //destruir o mutex antes de sair
            fechar_socket(client_fd);
            continue; //continua para aceitar novas conexões mesmo que uma falhe
        }

        
        aguardar_thread(thread_recebimento); //espera a thread de recebimento terminar
        aguardar_thread(thread_envio_periodico); //espera a thread de envio periodico
        
        destruir_mutex(&cliente.mutex); //destruir o mutex antes de sair

        fechar_socket(client_fd);
    }

    //fechar o socket do servidor, saida do loop ainda nao implementada
    fechar_socket(server_fd);
    finalizar_sockets(); //encerra o winsock que inicializamos no começo

    return 0;
}