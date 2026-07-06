# Cerveur

Microframework HTTP escrito em C.

## Estado atual

Até o momento, um servidor HTTP mínimo single-threaded projetado para ser Linux-first.

## Como rodar

- Alguns cabeçalhos, como o <arpa/inet.h>, só vão estar disponíveis em ambientes Linux. Caso o usuário queira rodar o servidor no Windows, seria necessário utilizar o WSL.

```powershell
wsl --install
```

Dentro do WSL:

```bash
sudo apt update
sudo apt upgrade -y
sudo apt install make gcc -y

make all
make run
```

Para limpar o executável do Cerveur:

```bash
make clean
```
