#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "http.h"

int analizar_peticion_http(const char *datos_recibidos, PeticionHTTP *peticion) {
    if (datos_recibidos == NULL || peticion == NULL) {
        return CODIGO_400_BAD_REQUEST;
    }

    memset(peticion, 0, sizeof(PeticionHTTP));

    if (strstr(datos_recibidos, "\r\n\r\n") == NULL) {
        return CODIGO_400_BAD_REQUEST;
    }

    const char *fin_primera_linea = strstr(datos_recibidos, "\r\n");
    if (fin_primera_linea == NULL) {
        return CODIGO_400_BAD_REQUEST;
    }

    if ((fin_primera_linea - datos_recibidos) >= 512) {
        return CODIGO_400_BAD_REQUEST;
    }

    int campos = sscanf(datos_recibidos, "%15s %255s %15s", peticion->metodo, peticion->uri, peticion->version);
    if (campos != 3) {
        return CODIGO_400_BAD_REQUEST;
    }

    if (strcmp(peticion->metodo, "GET") != 0) {
        return CODIGO_405_METHOD_NOT_ALLOWED;
    }

    if (peticion->uri[0] != '/' || strlen(peticion->uri) >= 250) {
        return CODIGO_400_BAD_REQUEST;
    }

    if (strcmp(peticion->version, "HTTP/1.1") != 0) {
        return CODIGO_400_BAD_REQUEST;
    }

    const char *linea_actual = fin_primera_linea + 2;

    while (linea_actual != NULL && !(linea_actual[0] == '\r' && linea_actual[1] == '\n')) {
        const char *siguiente_salto = strstr(linea_actual, "\r\n");
        if (siguiente_salto == NULL) {
            return CODIGO_400_BAD_REQUEST;
        }

        if (strncasecmp(linea_actual, "Host:", 5) == 0) {
            const char *valor = linea_actual + 5;
            while (*valor == ' ' || *valor == '\t') {
                valor++;
            }
            if (sscanf(valor, "%127s", peticion->host) == 1) {
                peticion->tiene_host = 1;
            }
        } else if (strncasecmp(linea_actual, "Connection:", 11) == 0) {
            const char *valor = linea_actual + 11;
            while (*valor == ' ' || *valor == '\t') {
                valor++;
            }
            if (sscanf(valor, "%31s", peticion->conexion) == 1) {
                peticion->tiene_conexion = 1;
            }
        } else if (strncasecmp(linea_actual, "User-Agent:", 11) == 0) {
            const char *valor = linea_actual + 11;
            while (*valor == ' ' || *valor == '\t') {
                valor++;
            }
            if (sscanf(valor, "%255[^\r\n]", peticion->user_agent) == 1) {
                peticion->tiene_user_agent = 1;
            }
        }

        linea_actual = siguiente_salto + 2;
    }

    if (peticion->tiene_host == 0 || peticion->tiene_conexion == 0 || peticion->tiene_user_agent == 0) {
        return CODIGO_400_BAD_REQUEST;
    }

    return CODIGO_200_OK;
}

const char *texto_codigo_estado(int codigo) {
    if (codigo == CODIGO_200_OK) {
        return "OK";
    }
    if (codigo == CODIGO_400_BAD_REQUEST) {
        return "Bad Request";
    }
    if (codigo == CODIGO_403_FORBIDDEN) {
        return "Forbidden";
    }
    if (codigo == CODIGO_404_NOT_FOUND) {
        return "Not Found";
    }
    if (codigo == CODIGO_405_METHOD_NOT_ALLOWED) {
        return "Method Not Allowed";
    }
    return "Internal Server Error";
}

int armar_cabecera_respuesta(int codigo, const char *mime, long longitud, const char *conexion, char *salida, int max_salida) {
    if (mime == NULL || conexion == NULL || salida == NULL || max_salida <= 0) {
        return -1;
    }

    const char *texto = texto_codigo_estado(codigo);

    int total = snprintf(
        salida,
        max_salida,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: %s\r\n"
        "\r\n",
        codigo,
        texto,
        mime,
        longitud,
        conexion
    );

    if (total < 0 || total >= max_salida) {
        return -1;
    }

    return total;
}

int armar_respuesta_error(int codigo, char *salida, int max_salida) {
    if (salida == NULL || max_salida <= 0) {
        return -1;
    }

    const char *texto = texto_codigo_estado(codigo);
    char html_pequeno[256];

    int largo_html = snprintf(
        html_pequeno,
        sizeof(html_pequeno),
        "<html><body><h1>%d %s</h1></body></html>",
        codigo,
        texto
    );

    if (largo_html < 0 || largo_html >= (int)sizeof(html_pequeno)) {
        return -1;
    }

    int total = snprintf(
        salida,
        max_salida,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        codigo,
        texto,
        largo_html,
        html_pequeno
    );

    if (total < 0 || total >= max_salida) {
        return -1;
    }

    return total;
}
