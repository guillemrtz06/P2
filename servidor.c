#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PUERTO 9999
#define TAM_BUFFER 1024

int iniciar_servidor(int puerto) {
    int conector = socket(AF_INET, SOCK_STREAM, 0);
    if (conector < 0) {
        perror("Error al crear el socket");
        exit(1);
    }

    int opcion = 1;
    setsockopt(conector, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion));

    struct sockaddr_in direccion;
    memset(&direccion, 0, sizeof(direccion));
    direccion.sin_family = AF_INET;
    direccion.sin_port = htons(puerto);
    direccion.sin_addr.s_addr = INADDR_ANY;

    if (bind(conector, (struct sockaddr *)&direccion, sizeof(direccion)) < 0) {
        perror("Error en bind");
        close(conector);
        exit(1);
    }

    if (listen(conector, 5) < 0) {
        perror("Error en listen");
        close(conector);
        exit(1);
    }

    return conector;
}

void enviar_archivo(int socket_cliente, const char *nombre_archivo) {
    FILE *archivo = fopen(nombre_archivo, "rb");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    char buffer[TAM_BUFFER];
    size_t bytes_leidos;

    while ((bytes_leidos = fread(buffer, 1, sizeof(buffer), archivo)) > 0) {
        if (write(socket_cliente, buffer, bytes_leidos) < 0) {
            perror("Error al enviar datos");
            break;
        }
    }

    fclose(archivo);
}

void atender_cliente(int socket_escucha, int socket_cliente) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Error en fork");
        close(socket_cliente);
    } 
    else if (pid == 0) {
        
        close(socket_escucha);
        enviar_archivo(socket_cliente, "Google.html");
        close(socket_cliente);
        exit(0);
    } 
    else {
        
        close(socket_cliente);
    }
}

int main() {
    signal(SIGCHLD, SIG_IGN);

    int conector = iniciar_servidor(PUERTO);
    printf("Servidor escuchando en el puerto %d...\n", PUERTO);

    while (1) {
        struct sockaddr_in dir_cliente;
        socklen_t tam_cliente = sizeof(dir_cliente);

        int nuevo_conector = accept(conector, (struct sockaddr *)&dir_cliente, &tam_cliente);
        if (nuevo_conector < 0) {
            perror("Error en accept");
            continue;
        }

        atender_cliente(conector, nuevo_conector);
    }

    close(conector);
    return 0;
}