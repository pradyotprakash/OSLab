#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> 
#include <string.h>
#include <pthread.h>

#define PACKETSIZE 512

void error(char *msg){
	perror(msg);
	exit(1);
}

struct sockaddr_in serv_addr;
struct hostent *server;
int portno, isRandom = 0;
float duration, think_time;
struct timespec start, finish;

unsigned long *requestsServed;
double *responseTimes;

void *_main(void* arg){
	struct timespec now, sendTime, receiveTime;
	double elapsed, respTime;
	int i = *((int*) arg);
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
			int r = rand()%10000;
			sprintf(buffer, "get files/foo%d.txt", r);
		}
		else{
			sprintf(buffer, "get files/foo0.txt");
		}

		// record the time before the request is sent
		clock_gettime(CLOCK_MONOTONIC, &sendTime);
		
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
		
		// record the time when file is received
		clock_gettime(CLOCK_MONOTONIC, &receiveTime);
		
		//calculate the total response time
		respTime = (receiveTime.tv_sec - sendTime.tv_sec);
		respTime += (receiveTime.tv_nsec - sendTime.tv_nsec) / 1000000000.0;

		// if(totalBytes > 0 && totalBytes < PACKETSIZE)
		// 	printf("%s\n", recvbuffer);
		// else if(totalBytes > 0)
		// 	printf("File Successfully received\n");
		// else if(totalBytes == 0){
		// 	printf("0 bytes received\n");
		// }
		
		close(sockfd);
		free(buffer);
		requestsServed[i]++;
		responseTimes[i] += respTime;
		
		// needed for checking when to stop the loop
		clock_gettime(CLOCK_MONOTONIC, &now);
		elapsed = (now.tv_sec - start.tv_sec);
		elapsed += (now.tv_nsec - start.tv_nsec) / 1000000000.0;

		if(elapsed > duration)
			break;

		usleep(think_time);	// wait for the specified amount of time
	}
}

int main(int argc, char *argv[]){
	
	int NUM_THREADS = 1;	// some default assignment

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

	// do this for the random mode of operation
	srand(time(NULL));
	
	// record the initial time when starting the experiments
	clock_gettime(CLOCK_MONOTONIC, &start);

	requestsServed = (unsigned long*) calloc(NUM_THREADS, sizeof(unsigned long));
	responseTimes = (double*) calloc(NUM_THREADS, sizeof(double)); 

	// create the necessary threads
	pthread_t threads[NUM_THREADS];
	int i;
	for(i=0;i<NUM_THREADS;++i){
		// create a thread
		int iret = pthread_create(&threads[i], NULL, _main, &i);	
		
		if(iret){
			fprintf(stderr, "Error - pthread_create() return code: %d\n", iret);
			exit(2);
		}
	}

	// make the parent wait for them to exit
	for(i=0;i<NUM_THREADS;++i){
		pthread_join(threads[i], NULL);
	}

	clock_gettime(CLOCK_MONOTONIC, &finish);

	// calculate throughput
	double totalSuccessfulRequests = 0.0;
	double totalTime = 0.0;
	
	for(i=0;i<NUM_THREADS;++i){
		totalSuccessfulRequests += requestsServed[i];
		totalTime += responseTimes[i];
	}

	double t = (finish.tv_sec - start.tv_sec);
	t += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	
	printf("Done!\n");
	printf("throughput = %f req/s\n", totalSuccessfulRequests/(t));
	printf("average response time = %f sec\n", totalTime/totalSuccessfulRequests);

}
