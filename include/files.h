#ifndef FILES_H
#define FILES_H

#define ARCHIVO_OK 0
#define ARCHIVO_ERROR -1
#define ARCHIVO_NO_EXISTE -2
#define ARCHIVO_PROHIBIDO -3

int validar_ruta_segura(const char *uri, char *ruta_resuelta, int max_largo);
int obtener_longitud_archivo(const char *ruta, long *longitud);
int leer_contenido_binario(const char *ruta, char *destino, long longitud, long *total_leido);

#endif
