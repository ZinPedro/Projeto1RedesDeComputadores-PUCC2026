#include <stdio.h>
#include <winsock2.h>

int main()
{
    int clientSocket;
    printf("Cliente iniciado!\n");

    clientSocket = socket(AF_INET, SOCK_STREAM, 0); 
    
    return 0;
}
