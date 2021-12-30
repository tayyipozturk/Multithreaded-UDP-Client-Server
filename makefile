cc=gcc
CFLAGS=-pthread
MAKE=make
RM=rm

server_target:
	$(cc) $(CFLAGS) server.c functions.c -o server
	./server 8080

client_target:
	$(cc) $(CFLAGS) client.c functions.c -o client
	./client 127.0.0.1 8080 8085

clean:server client
	$(RM) server
	$(RM) client