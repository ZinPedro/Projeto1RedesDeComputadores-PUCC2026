# Cliente-Servidor TCP - Redes de Computadores

Projeto acadêmico da disciplina de Redes de Computadores. Engenharia da Computação, PUC Campinas.

## Equipe

- Eloise Dos Santos Ruiz
- Lucas Leal Ibrahim
- Pedro Henrique Coan Zin

## Sobre o projeto

Sistema de comunicação cliente-servidor implementado em C utilizando sockets TCP.

O projeto permite estabelecer uma conexão entre um cliente e um servidor através da porta 8080, possibilitando o envio e recebimento de mensagens.

A aplicação utiliza threads para permitir que o cliente receba mensagens do servidor enquanto também pode enviar dados pelo terminal.

O projeto também possui uma camada de compatibilidade entre Windows e Linux, responsável por abstrair diferenças relacionadas a sockets, threads e mutex.

## Funcionalidades

- Comunicação cliente-servidor utilizando TCP
- Conexão através da porta 8080
- Envio de mensagens do cliente para o servidor
- Recebimento de mensagens do servidor
- Uso de threads para envio e recebimento simultâneo
- Uso de mutex para controle de dados compartilhados
- Envio periódico de data e hora pelo servidor
- Alteração do nome do usuário através do comando `:nome`
- Encerramento da conexão através do comando `:quit`
- Compatibilidade com Windows e Linux

## Funcionamento

O servidor cria um socket TCP e permanece aguardando conexões na porta `8080`.

Quando um cliente se conecta, o servidor envia uma mensagem confirmando a conexão:

```text
18:33: CONECTADO!!
```

Durante a conexão, o servidor também envia periodicamente a data e hora atual:

```text
22/08/2026 18:33
```

O cliente possui threads separadas para envio e recebimento, permitindo que mensagens sejam recebidas enquanto o usuário utiliza o terminal.

## Comandos

### Alterar nome

O usuário pode alterar o nome armazenado utilizando:

```text
:nome Antonio
```

O servidor processa o comando e atualiza o nome do usuário.

### Desconectar

Para encerrar a conexão:

```text
:quit
```

O cliente encerra sua conexão e o servidor volta a aguardar novos clientes.

### Enviar mensagem

Qualquer texto que não corresponda a um comando é tratado como uma mensagem.

Exemplo:

```text
oiii
```

O servidor processa a mensagem e atualmente retorna:

```text
Voce digitou: oiii
```

## Estrutura do projeto

```text
Projeto1RedesDeComputadores-PUCC2026/
│
├── include/
│   ├── platform.h
│   └── protocol.h
│
├── src/
│   ├── cliente.c
│   ├── server.c
│   └── protocol.c
│
└── README.md
```

O arquivo `cliente.c` contém a implementação do cliente TCP.

O arquivo `server.c` contém a implementação do servidor e o gerenciamento das conexões.

O arquivo `protocol.c` contém o processamento dos comandos e mensagens.

O arquivo `platform.h` implementa a camada de compatibilidade necessária para executar o projeto tanto no Windows quanto no Linux.

## Tecnologias

C, TCP/IP, sockets, threads, mutex, Winsock, POSIX e pthreads.


## Como rodar

Primeiro execute o servidor:

```powershell
.\server.exe
```

O servidor ficará aguardando conexões:

```text
Servidor aguardando conexoes na porta 8080...
```

Em outro terminal, execute o cliente:

```powershell
.\cliente.exe
```

Após a conexão, o cliente estará pronto para enviar e receber mensagens.

## Testes realizados

O projeto foi compilado e testado em ambiente Windows.

Durante os testes foram validados:

- Inicialização do servidor
- Conexão do cliente
- Envio e recebimento de mensagens
- Recebimento periódico de data e hora
- Alteração de nome com `:nome`
- Desconexão com `:quit`
- Retorno do servidor ao estado de espera após a desconexão do cliente

## Nível do projeto

O projeto aplica conceitos fundamentais de Redes de Computadores e programação concorrente em C, incluindo sockets TCP, comunicação cliente-servidor, threads, mutex e tratamento de diferenças entre sistemas operacionais.

A estrutura separa a comunicação, o protocolo e as funcionalidades específicas de cada plataforma, facilitando a organização e evolução do código.