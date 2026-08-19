#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <pthread.h>

#define MAX_NOME 64
#define MAX_MSG 512

// Criando uma enumeracao das acoes predefinidas que o usuario pode fazer 

typedef enum {

    ACAO_NENHUMA,
    ACAO_MUDAR_NOME,
    ACAO_ENVIAR_MSG,
    ACAO_DESCONECTAR,

} tipo_acao_t;

// Estrutura para a memória compartilhada entre as threads thread 1 (gravar/escrever) e a thread 2 (ler/processar) do servidor

typedef struct mem_compartilhada {

    tipo_acao_t acao; // acao que vai fazer
    char nome_usuario[MAX_NOME]; // nome do usuario para aparecer na mensagem
    char conteudo[MAX_MSG]; // a mensagem em si
    int pendente; // aviso se tem mensagem nova ou nao

} shared_data_t;

extern pthread_mutex_t lock; // mutex para controlar a condicao de corrida na memoria compartilhada

// funcao que vai receber a info do socket ve qual acao é, de quem é, qual msg,  grava o resultado na mem compartlhada

void parse_input(const char *linha, shared_data_t *shared_data); 

// funcao para o servidor processar o que estiver pendente, escreve o texto formatado para imprimir, retorna 1 se tiver processado algo, 0 se nao processou

int process_shared_data(shared_data_t *shared_data, char *saida, int *tam_saida);

#endif