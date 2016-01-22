all: multi-client.c server-mp.c
	gcc -w multi-client.c -o multi-client -lpthread
	gcc -w server-mp.c -o server-mp

clean:
	rm -f server-mp multi-client
