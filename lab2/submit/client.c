#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define PACKETSIZE 512

void error(char *msg){
	perror(msg);
	exit(1);
}

struct sockaddr_in serv_addr;
struct hostent *server;
int portno, isRandom = 0;
float duration, think_time;

void *_main(void* arg){
	while(1){
		int n, sockfd;
		
		/* create socket, get sockfd handle */
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			error("ERROR opening socket");
			
		/* connect to server */
		if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
			error("ERROR connecting");

		char *buffer = (char *) malloc(30);
		if(isRandom){
			int r = rand()%10;
			sprintf(buffer, "get files/foo%d.txt", r);
		}
		else{
			sprintf(buffer, "get files/foo0.txt");
		}

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
		free(buffer);
		break;
		usleep(think_time);
	}
}

int main(int argc, char *argv[]){
	
	int NUM_THREADS = 1;

	if (argc < 7) {
		fprintf(stderr, "usage %s hostname port #users duration_of_expt. think_time mode\n", argv[0]);
		exit(1);
	}

	portno = atoi(argv[2]);
	NUM_THREADS = atoi(argv[3]);
	duration = atof(argv[4]);
	think_time = atof(argv[5]);

	char* _random = argv[6];
	if(_random[0] == 'r' && _random[1] == 'a' && _random[2] == 'n' && _random[3] == 'd' && _random[4] == 'o' && _random[5] == 'm' && _random[6] == '\0')
		isRandom = 1;
	else if(_random[0] == 'f' && _random[1] == 'i' && _random[2] == 'x' && _random[3] == 'e' && _random[4] == 'd' && _random[5] == '\0')
		isRandom = 0;
	else{
		fprintf(stderr, "last argument can be only \"random\" or \"fixed\"\n");
		exit(1);
	}

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

	srand(time(NULL));
	
	// create the necessary threads
	pthread_t threads[NUM_THREADS];
	for(int i=0;i<NUM_THREADS;++i){
		pthread_create(&threads[i], NULL, _main, NULL);	
		pthread_join(threads[i], NULL);
	}
}
