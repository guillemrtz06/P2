#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PUERTO 9999
#define TAM_BUFFER 1024

void validar_argumentos(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s IP_Servidor\n", argv[0]);
        exit(1);
    }
}

int conectar_servidor(const char *ip_servidor, int puerto) {
    int conector_cliente = socket(AF_INET, SOCK_STREAM, 0);
    if (conector_cliente < 0) {
        perror("Error al crear el socket");
        exit(1);
    }

    struct sockaddr_in dir_servidor;
    memset(&dir_servidor, 0, sizeof(dir_servidor));
    dir_servidor.sin_family = AF_INET;
    dir_servidor.sin_port = htons(puerto);
    dir_servidor.sin_addr.s_addr = inet_addr(ip_servidor);

    if (dir_servidor.sin_addr.s_addr == INADDR_NONE) {
        fprintf(stderr, "Dirección IP no válida.\n");
        close(conector_cliente);
        exit(1);
    }

    if (connect(conector_cliente, (struct sockaddr *)&dir_servidor, sizeof(dir_servidor)) < 0) {
        perror("Error al conectar con el servidor");
        close(conector_cliente);
        exit(1);
    }

    return conector_cliente;
}

void recibir_archivo(int socket_servidor) {
    char buffer[TAM_BUFFER];
    ssize_t bytes_recibidos;

    while ((bytes_recibidos = read(socket_servidor, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_recibidos, stdout);
    }

    if (bytes_recibidos < 0) {
        perror("Error al recibir datos");
    }
}

int main(int argc, char *argv[]) {
    validar_argumentos(argc, argv);

    int conector_cliente = conectar_servidor(argv[1], PUERTO);
    recibir_archivo(conector_cliente);

    close(conector_cliente);
    return 0;
}