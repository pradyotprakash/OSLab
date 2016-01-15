#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define PACKETSIZE 512

void error(char *msg){
	perror(msg);
	exit(1);
}

struct sockaddr_in serv_addr;
struct hostent *server;
int sockfd, portno;

void *_main(void* arg){
	int n;
	
	/* create socket, get sockfd handle */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
		
	/* connect to server */
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");

	char *buffer = "get client.c";

	/* send user message to server */
	n = write(sockfd, buffer, strlen(buffer));
	if (n < 0)
		error("ERROR writing to socket");

	/* read reply from server */
	char recvbuffer[PACKETSIZE];
	int bytesRead;
	int totalBytes = 0;

	while((bytesRead = read(sockfd, recvbuffer, PACKETSIZE)) > 0){
		totalBytes += bytesRead; 
	}

	if (bytesRead < 0)
		error("ERROR reading from socket");
	
	if(totalBytes > 0 && totalBytes < PACKETSIZE)
		printf("%s\n", recvbuffer);
	else if(totalBytes > 0)
		printf("File Successfully received\n");
	else if(totalBytes == 0){
		printf("0 bytes received\n");
	}
	
	close(sockfd);
	return;
}

int main(int argc, char *argv[]){
	
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(1);
	}

	portno = atoi(argv[2]);

	/* fill in server address in sockaddr_in datastructure */
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	// create the necessary threads
	int NUM_THREADS = 10;
	pthread_t threads[NUM_THREADS];
	for(int i=0;i<NUM_THREADS;++i){
		pthread_create(&threads[i], NULL, _main, NULL);	
		pthread_join(threads[i], NULL);
	}
}
