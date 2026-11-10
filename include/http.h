#ifndef HTTP_H
#define HTTP_H

#define CODIGO_200_OK 200
#define CODIGO_400_BAD_REQUEST 400
#define CODIGO_403_FORBIDDEN 403
#define CODIGO_404_NOT_FOUND 404
#define CODIGO_405_METHOD_NOT_ALLOWED 405
#define CODIGO_500_INTERNAL_ERROR 500

typedef struct {
    char metodo[16];
    char uri[256];
    char version[16];
    char host[128];
    char conexion[32];
    char user_agent[256];
    int tiene_host;
    int tiene_conexion;
    int tiene_user_agent;
} PeticionHTTP;

int analizar_peticion_http(const char *datos_recibidos, PeticionHTTP *peticion);
const char *texto_codigo_estado(int codigo);

int armar_cabecera_respuesta(int codigo, const char *mime, long longitud, const char *conexion, char *salida, int max_salida);

int armar_respuesta_error(int codigo, char *salida, int max_salida);

#endif
