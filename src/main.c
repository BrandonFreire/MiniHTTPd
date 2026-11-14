#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "server.h"

#define LIMITE_EVENTOS 64

int main() {
    int puerto = 8080;

    int socket_servidor = iniciar_socket_servidor(puerto);
    if (socket_servidor == -1) {
        printf("No se pudo iniciar el servidor\n");
        return 1;
    }

    int fd_epoll = epoll_create1(0);
    if (fd_epoll == -1) {
        printf("Fallo en epoll\n");
        close(socket_servidor);
        return 1;
    }

    struct epoll_event ev_registro;
    struct epoll_event lista_eventos[LIMITE_EVENTOS];

    ev_registro.events = EPOLLIN;
    ev_registro.data.fd = socket_servidor;

    if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, socket_servidor, &ev_registro) == -1) {
        printf("Fallo al registrar el servidor en epoll\n");
        close(fd_epoll);
        close(socket_servidor);
        return 1;
    }

    printf("Servidor activo en el puerto %d\n", puerto);

    while (1) {
        int listos = epoll_wait(fd_epoll, lista_eventos, LIMITE_EVENTOS, -1);
        if (listos == -1) {
            continue;
        }

        for (int i = 0; i < listos; i++) {
            int fd_activo = lista_eventos[i].data.fd;

            if (fd_activo == socket_servidor) {
                int nuevo_cliente = aceptar_nueva_conexion(socket_servidor);
                if (nuevo_cliente == -1) {
                    continue;
                }

                struct epoll_event ev_cliente;
                ev_cliente.events = EPOLLIN;
                ev_cliente.data.fd = nuevo_cliente;

                if (epoll_ctl(fd_epoll, EPOLL_CTL_ADD, nuevo_cliente, &ev_cliente) == -1) {
                    close(nuevo_cliente);
                }
            } else {
                int debe_cerrar = procesar_cliente(fd_activo);
                if (debe_cerrar == 1) {
                    epoll_ctl(fd_epoll, EPOLL_CTL_DEL, fd_activo, NULL);
                    close(fd_activo);
                }
            }
        }
    }

    close(fd_epoll);
    close(socket_servidor);
    return 0;
}
