# MiniHTTPd_C - Servidor HTTP/1.1 Básico

Proyecto para la materia de Computación Distribuida (ICCD654). Consiste en la implementación en lenguaje C para Linux (Ubuntu) de un servidor web básico que soporta el protocolo HTTP/1.1 sin usar librerías externas.

El servidor maneja múltiples clientes en un solo hilo mediante multiplexación de E/S con `epoll`, mantiene conexiones persistentes (`keep-alive`), sirve archivos estáticos desde la carpeta `www/` y valida las peticiones entrantes.

## Organización de carpetas

```text
MiniHTTPd_C/
├── Makefile
├── README.md
├── include/
│   ├── files.h
│   ├── http.h
│   ├── mime.h
│   └── server.h
├── src/
│   ├── files.c
│   ├── http.c
│   ├── main.c
│   ├── mime.c
│   └── server.c
└── www/
    ├── index.html
    ├── style.css
    └── logo.png
```

## Descripción de los archivos

- **main.c**: Inicializa el socket en el puerto 8080 y arranca el ciclo principal con `epoll`.
- **server.c**: Configuración del socket TCP, manejo de eventos con `epoll`, ciclo de atención al cliente para mantener la conexión activa (`keep-alive`) y envío completo de bytes con `send()`.
- **http.c**: Lectura y validación de la petición `GET` (revisando que incluya las cabeceras `Host`, `Connection` y `User-Agent`), además de armar las cabeceras de respuesta y las páginas pequeñas de error.
- **files.c**: Validación de rutas con `realpath()` para impedir que se acceda a carpetas fuera de `www/`, obtención del tamaño con `stat()` y lectura de archivos en modo binario (`"rb"`).
- **mime.c**: Identificación de la extensión del archivo solicitado para devolver su `Content-Type` (`.html`, `.css`, `.js`, `.png`, `.jpg`).

## Compilación y ejecución

Para compilar el proyecto desde la terminal de Linux:

```bash
make
```

Para iniciar el servidor:

```bash
./minihttpd
```

También se puede compilar y correr directamente con:

```bash
make run
```

Para borrar el binario compilado:

```bash
make clean
```

## Pruebas realizadas

Con el servidor corriendo en el puerto `8080`, se puede abrir `http://localhost:8080/` en el navegador o probar desde otra terminal con `curl`:

**1. Petición GET válida (200 OK):**
```bash
curl -i -H "Connection: keep-alive" http://localhost:8080/
```

**2. Petición GET incompleta sin cabecera Connection (400 Bad Request):**
```bash
curl -i http://localhost:8080/
```

**3. Método no soportado como POST (405 Method Not Allowed):**
```bash
curl -i -X POST http://localhost:8080/
```

**4. Prueba de Directory Traversal saliendo de www (403 Forbidden):**
```bash
curl -i --path-as-is -H "Connection: keep-alive" http://localhost:8080/../../etc/passwd
```

**5. Recurso que no existe (404 Not Found):**
```bash
curl -i -H "Connection: keep-alive" http://localhost:8080/archivo_falso.html
```

**6. Cierre de conexión explícito:**
```bash
curl -i -H "Connection: close" http://localhost:8080/index.html
```
