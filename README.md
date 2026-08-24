# Cliente-Servidor TCP - Redes de Computadores

Projeto acadêmico da disciplina de Redes de Computadores — Engenharia da Computação, PUC Campinas.

## Equipe

- Eloise Dos Santos Ruiz
- Lucas Leal Ibrahim
- Pedro Henrique Coan Zin

## Sobre o projeto

Sistema de comunicação cliente-servidor implementado em C utilizando sockets TCP.

O projeto permite estabelecer uma conexão entre um cliente e um servidor através da porta `8080`, possibilitando o envio e recebimento de mensagens.

A aplicação utiliza threads para permitir comunicação bidirecional assíncrona e possui uma camada de compatibilidade entre Windows e Linux, responsável por abstrair diferenças de sockets, threads, mutex e outras funções dependentes do sistema operacional.

## Funcionalidades

- Comunicação cliente-servidor utilizando TCP
- Conexão através da porta `8080`
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

```text
:nome Antonio
```

### Desconectar

```text
:quit
```

### Enviar mensagem

Qualquer texto que não corresponda a um comando é tratado como mensagem.

Exemplo:

```text
oiii
```

O servidor retorna ao remetente:

```text
Voce digitou: oiii
```

## Estrutura do projeto

```text
Projeto1RedesDeComputadores-PUCC2026/
│
├── include/
│   ├── platform.h
│   ├── protocol.h
│   └── utils.h
│
├── src/
│   ├── cliente.c
│   ├── server.c
│   ├── protocol.c
│   └── utils.c
│
└── README.md
```

- `cliente.c`: implementação do cliente TCP.
- `server.c`: implementação do servidor e gerenciamento da conexão.
- `protocol.c` / `protocol.h`: interpretação e processamento dos comandos e mensagens.
- `platform.h`: camada de compatibilidade entre Windows e Linux.
- `utils.c` / `utils.h`: funções auxiliares do projeto.

## Tecnologias

C, TCP/IP, sockets, threads, mutex, Winsock, POSIX e pthreads.

# Como rodar

## Linux

### 1. Pré-requisitos

É necessário ter o GCC e as ferramentas básicas de compilação instalados.

No Ubuntu/Debian:

```bash
sudo apt update
sudo apt install build-essential
```

Verifique a instalação:

```bash
gcc --version
```

### 2. Compilar o servidor

Na pasta raiz do projeto:

```bash
gcc src/server.c src/protocol.c -Iinclude -o server -pthread -Wall -Wextra
```

### 3. Compilar o cliente

```bash
gcc src/cliente.c -Iinclude -o cliente -pthread -Wall -Wextra
```

### 4. Executar

Abra dois terminais na pasta do projeto.

Terminal 1 — servidor:

```bash
./server
```

Terminal 2 — cliente:

```bash
./cliente
```

O servidor deverá mostrar:

```text
Servidor aguardando conexoes na porta 8080...
```

---

## Windows

### 1. Pré-requisitos

É necessário ter um compilador GCC para Windows, como o MinGW-w64.

Uma forma de instalar é através do MSYS2.

No PowerShell:

```powershell
winget install -e --id MSYS2.MSYS2
```

Depois abra o aplicativo **MSYS2 UCRT64** e execute:

```bash
pacman -Syu
```

Se o terminal solicitar reinicialização, feche o MSYS2 UCRT64, abra novamente e execute:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

Verifique no MSYS2:

```bash
gcc --version
```

### 2. Disponibilizar o GCC no PowerShell

Para poder usar `gcc` diretamente no PowerShell, adicione esta pasta ao `Path` do Windows:

```text
C:\msys64\ucrt64\bin
```

Caminho no Windows:

**Pesquisar "Variáveis de ambiente" → Variáveis de Ambiente → Path → Editar → Novo**

Adicione:

```text
C:\msys64\ucrt64\bin
```

Feche e abra novamente o PowerShell e confirme:

```powershell
gcc --version
```

### 3. Compilar o servidor

No PowerShell, na pasta raiz do projeto:

```powershell
gcc src/server.c src/protocol.c -Iinclude -o server.exe -lws2_32 -Wall -Wextra
```

### 4. Compilar o cliente

```powershell
gcc src/cliente.c -Iinclude -o cliente.exe -lws2_32 -Wall -Wextra
```

### 5. Executar

Abra dois PowerShells na pasta do projeto.

PowerShell 1 — servidor:

```powershell
.\server.exe
```

PowerShell 2 — cliente:

```powershell
.\cliente.exe
```

---

## Teste entre dois computadores

O cliente está configurado por padrão para:

```c
inet_addr("127.0.0.1")
```

Esse endereço funciona apenas quando cliente e servidor estão no mesmo computador.

Para utilizar dois computadores diferentes, descubra o IP da máquina que executará o servidor.

No Linux:

```bash
hostname -I
```

Exemplo:

```text
192.168.0.15
```

No `src/cliente.c`, altere:

```c
serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
```

para o IP do servidor:

```c
serverAddr.sin_addr.s_addr = inet_addr("192.168.0.15");
```

Depois recompile o cliente.

No Windows, com o servidor já executando, é possível testar a porta com:

```powershell
Test-NetConnection 192.168.0.15 -Port 8080
```

O resultado esperado é:

```text
TcpTestSucceeded : True
```

Os computadores podem estar conectados por Wi-Fi ou cabo, desde que estejam na mesma rede local e a comunicação entre dispositivos não esteja bloqueada pelo roteador ou firewall.

# Possíveis problemas

## `gcc` não é reconhecido no Windows

Mensagem semelhante a:

```text
gcc : O termo 'gcc' não é reconhecido...
```

Verifique primeiro:

```powershell
gcc --version
```

Se o GCC já estiver instalado através do MSYS2, confirme que esta pasta foi adicionada ao `Path`:

```text
C:\msys64\ucrt64\bin
```

Depois feche e abra novamente o PowerShell.

---

## `gcc: command not found` no Linux

Instale as ferramentas de compilação:

```bash
sudo apt update
sudo apt install build-essential
```

Depois verifique:

```bash
gcc --version
```

---

## Windows bloqueia `cliente.exe` ou `server.exe`

Se o PowerShell informar que uma política de Controle de Aplicativo bloqueou o executável, tente:

```powershell
Unblock-File .\cliente.exe
```

ou:

```powershell
Unblock-File .\server.exe
```

Depois tente executar novamente.

Se apenas o cliente continuar bloqueado, ele pode ser recompilado com outro nome:

```powershell
gcc src/cliente.c -Iinclude -o cliente_teste.exe -lws2_32 -Wall -Wextra
.\cliente_teste.exe
```

Se a política continuar bloqueando o programa, a restrição é do próprio Windows ou da configuração administrativa da máquina.

---

## Porta 8080 não está acessível

No Linux, confirme se o servidor está escutando:

```bash
ss -ltn | grep 8080
```

Se o `ufw` estiver ativo:

```bash
sudo ufw status
```

Caso necessário, libere a porta TCP 8080:

```bash
sudo ufw allow 8080/tcp
```

No Windows, para testar um servidor remoto:

```powershell
Test-NetConnection IP_DO_SERVIDOR -Port 8080
```

---

## `Address already in use`

Esse erro normalmente indica que outro processo já está utilizando a porta `8080`.

No Linux:

```bash
ss -ltnp | grep 8080
```

No Windows:

```powershell
netstat -ano | findstr :8080
```

Feche o processo anterior antes de executar uma nova instância do servidor.

---

## Cliente não conecta ao servidor em outro computador

Verifique:

1. Se os dois computadores estão na mesma rede local.
2. Se o endereço IP configurado no `cliente.c` é o IP da máquina do servidor.
3. Se o servidor está executando antes do cliente.
4. Se a porta `8080` está liberada no firewall.
5. Se a rede Wi-Fi não possui isolamento de clientes ou rede de convidados.

# Testes realizados

Durante os testes foram validados:

- Compilação e execução em Linux
- Compilação e execução em Windows
- Comunicação entre cliente e servidor
- Comunicação entre computadores diferentes na mesma rede local
- Envio e recebimento de mensagens
- Recebimento periódico de data e hora
- Alteração de nome com `:nome`
- Desconexão com `:quit`
- Retorno do servidor ao estado de espera após a desconexão do cliente

## Nível do projeto

O projeto aplica conceitos fundamentais de Redes de Computadores e programação concorrente em C, incluindo sockets TCP, comunicação cliente-servidor, threads, mutex e tratamento de diferenças entre sistemas operacionais.

A estrutura separa comunicação, protocolo e funcionalidades específicas de cada plataforma, facilitando a organização e a evolução do código.