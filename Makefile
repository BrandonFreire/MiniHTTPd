CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/server.c src/http.c src/files.c src/mime.c
TARGET = minihttpd

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)
