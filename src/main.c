#include <stdio.h>
#include <unistd.h>
#include "server.h"

int main() {
    int puerto = 8080;

    int socket_servidor = iniciar_socket_servidor(puerto);
    if (socket_servidor == -1) {
        printf("No se pudo iniciar el servidor\n");
        return 1;
    }

    printf("Servidor activo en el puerto %d\n", puerto);

    if (iniciar_bucle_epoll(socket_servidor) == -1) {
        printf("Fallo en epoll\n");
        close(socket_servidor);
        return 1;
    }

    close(socket_servidor);
    return 0;
}
