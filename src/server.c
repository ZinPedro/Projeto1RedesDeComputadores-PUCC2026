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

int main(){

    int server_fd;
    int client_fd;

    //vetor para armazenar a hora e mensagem de conexao (11:23: CONECTADO!!)
    char mensagem[64];
    //vetor para armazenar a mensagem recebida do cliente
    char buffer[BUFFER_SIZE];

    //informações do servidor e do cliente
    struct sockaddr_in server_addr = {0};
    struct sockaddr_in client_addr = {0};
    socklen_t client_addr_len = sizeof(client_addr); //tamanho do endereço do cliente

    //variável para armazenar o número de bytes recebidos do cliente
    ssize_t bytes_recebidos;

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

    printf("Servidor aguardando conexoes na porta %d...\n", PORT);

    //configuração do accept (socket, endereço do cliente, tamanho do endereço do cliente)
    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);

    if (client_fd < 0) {
        perror("Erro no accept");
        close(server_fd);
        return 1;
    }   

    printf("Conexao aceita de %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    //obter a hora atual
    time_t agora = time(NULL);
    struct tm *horario;
    time(&agora);
    horario = localtime(&agora);

    //Montar mensagem
    strftime(mensagem, sizeof(mensagem), "%H:%M: CONECTADO!!\n", horario);

    //enviar mensagem para o cliente
    printf("Enviando ao Cliente: %s", mensagem);
    //configuração do send (socket, mensagem, tamanho da mensagem, flags)
    if(send(client_fd, mensagem, strlen(mensagem), 0) < 0){
        perror("Erro ao enviar mensagem");
        close(client_fd);
        close(server_fd);
        return 1;
    }

    //receber mensagem do cliente
    //configuração do recv (socket, buffer, tamanho do buffer, flags)
    while(1){
        bytes_recebidos = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if(bytes_recebidos < 0) {
            perror("Erro ao receber mensagem");
            break;
        }

        if(bytes_recebidos == 0) {
            printf("Cliente desconectado.\n");
            break;
        } else {
            //verifica o tamanho da mensagem recebida e adiciona o terminador de string
            buffer[bytes_recebidos] = '\0'; // Adiciona o terminador de string
            printf("Mensagem recebida do cliente: %s\n", buffer);
        }
    }

    close(client_fd);
    close(server_fd);

    return 0;
}