#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>
#include "files.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

int validar_ruta_segura(const char *uri, char *ruta_resuelta, int max_largo) {
    if (uri == NULL || ruta_resuelta == NULL || max_largo <= 0) {
        return ARCHIVO_ERROR;
    }

    if (uri[0] != '/') {
        return ARCHIVO_ERROR;
    }

    const char *recurso = uri;
    if (strcmp(uri, "/") == 0) {
        recurso = "/index.html";
    }

    char base_www[PATH_MAX];
    char ruta_candidata[PATH_MAX];
    char ruta_real[PATH_MAX];

    if (realpath("www", base_www) == NULL) {
        return ARCHIVO_ERROR;
    }

    int escritos = snprintf(ruta_candidata, sizeof(ruta_candidata), "www%s", recurso);
    if (escritos < 0 || escritos >= (int)sizeof(ruta_candidata)) {
        return ARCHIVO_ERROR;
    }

    if (realpath(ruta_candidata, ruta_real) == NULL) {
        if (strstr(uri, "..") != NULL) {
            return ARCHIVO_PROHIBIDO;
        }
        if (errno == ENOENT) {
            return ARCHIVO_NO_EXISTE;
        }
        return ARCHIVO_ERROR;
    }

    size_t largo_base = strlen(base_www);
    if (strncmp(ruta_real, base_www, largo_base) != 0) {
        return ARCHIVO_PROHIBIDO;
    }

    if (ruta_real[largo_base] != '/' && ruta_real[largo_base] != '\0') {
        return ARCHIVO_PROHIBIDO;
    }

    int copiados = snprintf(ruta_resuelta, max_largo, "%s", ruta_real);
    if (copiados < 0 || copiados >= max_largo) {
        return ARCHIVO_ERROR;
    }

    return ARCHIVO_OK;
}

int obtener_longitud_archivo(const char *ruta, long *longitud) {
    if (ruta == NULL || longitud == NULL) {
        return ARCHIVO_ERROR;
    }

    struct stat datos_archivo;
    if (stat(ruta, &datos_archivo) != 0) {
        if (errno == ENOENT) {
            return ARCHIVO_NO_EXISTE;
        }
        return ARCHIVO_ERROR;
    }

    if (!S_ISREG(datos_archivo.st_mode)) {
        return ARCHIVO_PROHIBIDO;
    }

    *longitud = (long)datos_archivo.st_size;
    return ARCHIVO_OK;
}

int leer_contenido_binario(const char *ruta, char *destino, long longitud, long *total_leido) {
    if (ruta == NULL || destino == NULL || total_leido == NULL || longitud < 0) {
        return ARCHIVO_ERROR;
    }

    FILE *archivo = fopen(ruta, "rb");
    if (archivo == NULL) {
        return ARCHIVO_ERROR;
    }

    size_t leidos = fread(destino, 1, (size_t)longitud, archivo);
    if (ferror(archivo)) {
        fclose(archivo);
        return ARCHIVO_ERROR;
    }

    fclose(archivo);

    if ((long)leidos != longitud) {
        return ARCHIVO_ERROR;
    }

    *total_leido = (long)leidos;
    return ARCHIVO_OK;
}
