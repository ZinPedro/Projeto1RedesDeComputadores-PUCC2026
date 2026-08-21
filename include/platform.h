#ifndef PLATFORM_H
#define PLATFORM_H

#include <time.h>
#include <stdio.h>

//Blibliotecas para Windows
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>

    typedef SOCKET socket_t;
    typedef int socket_io_t; // Definir socklen_t como int no Windows
    typedef int sock_len_t; // Definir socklen_t como int no Windows

    #define PLATFORM_SOCKET_INVALIDO INVALID_SOCKET
    #define PLATFORM_SOCKET_ERRO SOCKET_ERROR
    #define PLARFORM_SHUT_RDWR SD_BOTH

#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>

    typedef int socket_t;
    typedef ssize_t socket_io_t; // Definir socklen_t como int no Windows
    typedef socklen_t sock_len_t; // Definir socklen_t como int no Windows

    #define PLATFORM_SOCKET_INVALIDO (-1)
    #define PLATFORM_SOCKET_ERRO (-1)
    #define PLATFORM_SHUT_RDWR SHUT_RDWR

#endif

//inixializa o socket de acordo com o SO
static inline int iniciar_sockets(void){
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printf("Erro ao iniciar o Winsock\n");
            return 0;
        }
    #endif
    return 1;
}

//finaliza o socket de acordo com o SO
static inline void finalizar_sockets(void){
    #ifdef _WIN32
        WSACleanup();
    #endif
}

//fecha o socket de acordo com o SO
static inline void fechar_socket(socket_t socket_fd){
    #ifdef _WIN32
        closesocket(socket_fd);
    #else
        close(socket_fd);
    #endif
}

//interrompe leitura e escrita no socket
static inline void desligar_socket(socket_t socket_fd){
        shutdown(socket_fd, PLATFORM_SHUT_RDWR);
}

//função para mostrar erro de socket de acordo com o SO
static inline void mostrar_erro_socket(const char *mensagem){
    #ifdef _WIN32
        fprintf(stderr, "%s. Código Winsock: %d\n", mensagem, WSAGetLastError());
    #else
        perror(mensagem);
    #endif
}

//função para dormir por um número de segundos de acordo com o SO
static inline void dormir_segundos(unsigned int segundos){
    #ifdef _WIN32
        Sleep(segundos * 1000);
    #else
        sleep(segundos);
    #endif
}

//função para obter o horário local de acordo com o SO
static inline int obter_horario_local(const time_t *tempo, struct tm *resultado){
    #ifdef _WIN32
        return localtime_s(resultado, tempo) == 0;
    #else
        return localtime_r(tempo, resultado) != NULL;
    #endif
}
#endif // PLATFORM_H
