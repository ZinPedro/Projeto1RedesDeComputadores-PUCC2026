#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/platform.h"

#define PORT 8080

int main()
{
 
        socket_t clientSocket;

        struct sockaddr_in serverAddr = {0};

        printf("Cliente iniciado!\n");

        //inicializa o winsock no Windows
        if (!iniciar_sockets()) {
            return 1;
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (clientSocket == PLATFORM_SOCKET_INVALIDO)
        {
            mostrar_erro_socket("Erro ao criar socket");
            finalizar_sockets(); //encerra o winsock que inicializamos no começo
            return 1;
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == PLATFORM_SOCKET_ERRO)
        {
            mostrar_erro_socket("Erro ao conectar ao servidor");
            fechar_socket(clientSocket); //fecha o socket se a conexão falhar
            finalizar_sockets(); //encerra o winsock que inicializamos no começo
            return 1;
        }

        printf("conectado ao servidor\n");

        fechar_socket(clientSocket); //fecha o socket após a conexão
        finalizar_sockets(); //encerra o winsock que inicializamos no começo

    return 0;


}