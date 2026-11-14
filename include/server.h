#ifndef SERVER_H
#define SERVER_H

int enviar_bytes_completos(int socket_cliente, const char *datos, long cantidad_total);
int iniciar_socket_servidor(int puerto);
int aceptar_nueva_conexion(int socket_servidor);
int procesar_cliente(int socket_cliente);

#endif
