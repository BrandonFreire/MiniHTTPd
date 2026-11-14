#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "server.h"
#include "http.h"
#include "files.h"
#include "mime.h"

#define TAMANO_BUFFER_RED 4096

int enviar_bytes_completos(int socket_cliente, const char *datos, long cantidad_total) {
    long enviados = 0;

    while (enviados < cantidad_total) {
        ssize_t n = send(socket_cliente, datos + enviados, cantidad_total - enviados, 0);
        if (n <= 0) {
            return -1;
        }
        enviados += n;
    }

    return 0;
}

int iniciar_socket_servidor(int puerto) {
    int fd_servidor = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_servidor == -1) {
        return -1;
    }

    int reutilizar = 1;
    if (setsockopt(fd_servidor, SOL_SOCKET, SO_REUSEADDR, &reutilizar, sizeof(reutilizar)) == -1) {
        close(fd_servidor);
        return -1;
    }

    struct sockaddr_in dir_servidor;
    memset(&dir_servidor, 0, sizeof(dir_servidor));
    dir_servidor.sin_family = AF_INET;
    dir_servidor.sin_addr.s_addr = INADDR_ANY;
    dir_servidor.sin_port = htons(puerto);

    if (bind(fd_servidor, (struct sockaddr *)&dir_servidor, sizeof(dir_servidor)) == -1) {
        close(fd_servidor);
        return -1;
    }

    if (listen(fd_servidor, 10) == -1) {
        close(fd_servidor);
        return -1;
    }

    return fd_servidor;
}

int aceptar_nueva_conexion(int socket_servidor) {
    struct sockaddr_in dir_cliente;
    socklen_t tamano_dir = sizeof(dir_cliente);

    int fd_cliente = accept(socket_servidor, (struct sockaddr *)&dir_cliente, &tamano_dir);
    if (fd_cliente == -1) {
        return -1;
    }

    return fd_cliente;
}

int procesar_cliente(int socket_cliente) {
    char buffer_entrada[TAMANO_BUFFER_RED];
    char buffer_salida[TAMANO_BUFFER_RED];
    PeticionHTTP peticion;
    int seguir_conectado = 1;

    while (seguir_conectado) {
        ssize_t recibidos = recv(socket_cliente, buffer_entrada, sizeof(buffer_entrada) - 1, MSG_DONTWAIT);

        if (recibidos == 0) {
            return 1;
        }
        if (recibidos < 0) {
            return 0;
        }

        buffer_entrada[recibidos] = '\0';

        int estado_validacion = analizar_peticion_http(buffer_entrada, &peticion);
        if (estado_validacion != CODIGO_200_OK) {
            int largo_err = armar_respuesta_error(estado_validacion, buffer_salida, sizeof(buffer_salida));
            if (largo_err > 0) {
                enviar_bytes_completos(socket_cliente, buffer_salida, largo_err);
            }
            return 1;
        }

        char ruta_segura[1024];
        int res_ruta = validar_ruta_segura(peticion.uri, ruta_segura, sizeof(ruta_segura));
        if (res_ruta != ARCHIVO_OK) {
            int codigo_err = CODIGO_500_INTERNAL_ERROR;
            if (res_ruta == ARCHIVO_PROHIBIDO) {
                codigo_err = CODIGO_403_FORBIDDEN;
            } else if (res_ruta == ARCHIVO_NO_EXISTE) {
                codigo_err = CODIGO_404_NOT_FOUND;
            }
            int largo_err = armar_respuesta_error(codigo_err, buffer_salida, sizeof(buffer_salida));
            if (largo_err > 0) {
                enviar_bytes_completos(socket_cliente, buffer_salida, largo_err);
            }
            return 1;
        }

        long tamano_archivo = 0;
        int res_tamano = obtener_longitud_archivo(ruta_segura, &tamano_archivo);
        if (res_tamano != ARCHIVO_OK) {
            int codigo_err = CODIGO_500_INTERNAL_ERROR;
            if (res_tamano == ARCHIVO_PROHIBIDO) {
                codigo_err = CODIGO_403_FORBIDDEN;
            } else if (res_tamano == ARCHIVO_NO_EXISTE) {
                codigo_err = CODIGO_404_NOT_FOUND;
            }
            int largo_err = armar_respuesta_error(codigo_err, buffer_salida, sizeof(buffer_salida));
            if (largo_err > 0) {
                enviar_bytes_completos(socket_cliente, buffer_salida, largo_err);
            }
            return 1;
        }

        const char *mime = obtener_mime(ruta_segura);
        if (mime == NULL) {
            int largo_err = armar_respuesta_error(CODIGO_500_INTERNAL_ERROR, buffer_salida, sizeof(buffer_salida));
            if (largo_err > 0) {
                enviar_bytes_completos(socket_cliente, buffer_salida, largo_err);
            }
            return 1;
        }

        char *buffer_archivo = NULL;
        long bytes_archivo = 0;

        if (tamano_archivo > 0) {
            buffer_archivo = malloc(tamano_archivo);
            if (buffer_archivo == NULL) {
                int largo_err = armar_respuesta_error(CODIGO_500_INTERNAL_ERROR, buffer_salida, sizeof(buffer_salida));
                if (largo_err > 0) {
                    enviar_bytes_completos(socket_cliente, buffer_salida, largo_err);
                }
                return 1;
            }

            if (leer_contenido_binario(ruta_segura, buffer_archivo, tamano_archivo, &bytes_archivo) != ARCHIVO_OK) {
                free(buffer_archivo);
                int largo_err = armar_respuesta_error(CODIGO_500_INTERNAL_ERROR, buffer_salida, sizeof(buffer_salida));
                if (largo_err > 0) {
                    enviar_bytes_completos(socket_cliente, buffer_salida, largo_err);
                }
                return 1;
            }
        }

        const char *tipo_conexion = "keep-alive";
        if (strcasecmp(peticion.conexion, "close") == 0) {
            tipo_conexion = "close";
        }

        int largo_cabecera = armar_cabecera_respuesta(
            CODIGO_200_OK,
            mime,
            tamano_archivo,
            tipo_conexion,
            buffer_salida,
            sizeof(buffer_salida)
        );

        if (largo_cabecera < 0) {
            free(buffer_archivo);
            return 1;
        }

        if (enviar_bytes_completos(socket_cliente, buffer_salida, largo_cabecera) != 0) {
            free(buffer_archivo);
            return 1;
        }

        if (tamano_archivo > 0) {
            if (enviar_bytes_completos(socket_cliente, buffer_archivo, tamano_archivo) != 0) {
                free(buffer_archivo);
                return 1;
            }
        }

        free(buffer_archivo);

        if (strcmp(peticion.metodo, "GET") == 0 && strcasecmp(peticion.conexion, "close") != 0) {
            seguir_conectado = 1;
        } else {
            seguir_conectado = 0;
        }
    }

    return 1;
}