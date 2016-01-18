all: client.c server.c
	gcc -w client.c -o multi-client -lpthread
	gcc -w server.c -o server-mp

clean:
	rm -f server-mp multi-client	