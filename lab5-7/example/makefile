CC = g++
CFLAGS = -Wall -Wextra -pedantic -Werror
ZMQ = `pkg-config --libs libzmq`

all: client server

client: client.cpp
	$(CC) $(CFLAGS) client.cpp -o client $(ZMQ)

server: server.cpp
	$(CC) $(CFLAGS) server.cpp -o server $(ZMQ)

clean:
	rm -f client server
