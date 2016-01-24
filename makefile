all: server client
server: server.c
	gcc -o server server.c -lssl -lcrypto -ldl
client: client.c
	gcc -o client client.c -lssl -lcrypto -ldl

