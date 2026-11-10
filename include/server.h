#ifndef SERVER_H
#define SERVER_H

int iniciar_socket_servidor(int puerto);
int aceptar_nueva_conexion(int socket_servidor);
int procesar_cliente(int socket_cliente);
int iniciar_bucle_epoll(int socket_servidor);

#endif
