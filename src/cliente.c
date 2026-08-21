#include <stdio.h>
#include <winsock2.h>

#define PORT 8080

int main()
{
    SOCKET clientSocket;
    printf("Cliente iniciado!\n");

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("erro ao iniciar o socket\n");
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == INVALID_SOCKET)
    {
        printf("erro ao criar o socket\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        printf("erro ao conectar ao servidor\n");
        closesocket(clientSocket); //fecha o socket se a conexão falhar
        WSACleanup(); //encerra o winsock que inicializamos no começo
        return 1;
    }

    printf("conectado ao servidor\n");

    closesocket(clientSocket); //fecha o socket
    WSACleanup(); //encerra o winsock

    return 0;
}