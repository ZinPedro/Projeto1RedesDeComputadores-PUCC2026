#include "protocol.h"
#include <string.h>
#include <stdio.h>

mutex_t lock;

void protocol_init() 
{
    iniciar_mutex(&lock); // funcao para inicializar o mutex (por conta da multiplataforma usei essa funcao que generaliza para windows ou linux)
}

void parse_input(const char *linha, shared_data_t *shared_data) 
{

    tipo_acao_t acao;
    char conteudo [MAX_MSG];

    //indentifica a ação recebida
    if(strncmp(linha, ":nome ", 6) == 0) // cada if aqui checa ara ver qual acao foi executada (olha se foi nome, desconectar ou enviar mensagem)
    {
        acao = ACAO_MUDAR_NOME; // passo a acao mudar nome para a acao na mem compartilhada para o servidor saber qual é a acao

        strncpy (conteudo, linha + 6, MAX_MSG - 1); // copia o nome
        conteudo [MAX_MSG - 1] = '\0';  // parar no \0 que é onde acabou a string
    }
    else if (strncmp(linha, ":quit", 5) == 0) 
    {
        acao = ACAO_DESCONECTAR;
        conteudo[0] = '\0';
    } 
    else 
    {
        acao = ACAO_ENVIAR_MSG;
        strncpy(conteudo, linha, MAX_MSG - 1);
        conteudo[MAX_MSG - 1] = '\0';
    }

    bloquear_mutex(&lock); // Tranco o mutex porque vamos escrever na mem compartilhada

    //verifica fila cheia
    if(shared_data->quantidade >= MAX_FILA){
        fprintf(stderr, "Fila de acoes cheia\n");
        liberar_mutex(&lock);
        return;
    }

    shared_data->fila[shared_data->fim].acao = acao; //adciona ação na fila

    strncpy(shared_data->fila[shared_data->fim].conteudo, conteudo, MAX_MSG - 1); //adciona conteudo da ação na fila

    shared_data->fila[shared_data->fim].conteudo[MAX_MSG - 1] = '\0'; //adciona sinalizador de fim da string 
    shared_data->fim = (shared_data->fim + 1) % MAX_FILA;   //altera posição do fim da fila
    shared_data->quantidade++; //registra ação no numero de ações

    liberar_mutex(&lock); // destranca o mutex
}

tipo_acao_t process_shared_data(shared_data_t *shared_data, char *saida, int tam_saida) 
{
    tipo_acao_t acao_processada;

    bloquear_mutex(&lock);

    if (!shared_data->pendente) // se pendente for diferente de 1, portanto nao tem nada pendente para ele processar, libera para a outra thread
    {
        liberar_mutex(&lock);
        saida[0] = '\0';
        return ACAO_NENHUMA;
    }

    acao_processada = shared_data->acao; // ve qual acao foi feita 

    switch (acao_processada) // switch case para checar todos os caso
    { 
        case ACAO_MUDAR_NOME:
            strncpy(shared_data->nome_usuario, shared_data->conteudo, MAX_NOME - 1);
            shared_data->nome_usuario[MAX_NOME - 1] = '\0';
            printf("DEBUG: nome atualizado para '%s'\n", shared_data->nome_usuario); // remover depois
            saida[0] = '\0';
            break;

        case ACAO_ENVIAR_MSG:
            snprintf(saida, tam_saida, "Voce digitou: %s\n", shared_data->conteudo);
            break;

        case ACAO_DESCONECTAR:
            saida[0] = '\0';
            break;

        default:
            saida[0] = '\0';
            break;
    }

    shared_data->acao = ACAO_NENHUMA;
    shared_data->pendente = 0;

    liberar_mutex(&lock);

    return acao_processada;
}

