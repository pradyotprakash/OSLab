/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

#define PACKETSIZE 512

int n_threads;
pthread_mutex_t mutex;
pthread_cond_t has_data, has_space;

void error(char *msg){
	perror(msg);
	exit(1);
}

struct queue{
	struct node *head, *tail;
	int max_len;
	int curr_len;
} waiting;

struct node{
	int val;
	struct node* next;
};

void add(struct queue *Q, int fd){

	struct node * n = (struct node*) malloc(sizeof(struct node));
	n->val = fd;
	while(Q->curr_len == Q->max_len && Q->max_len!=0){
		pthread_cond_wait(&has_space, &mutex);
	}

	if(Q->head == NULL){
		Q->head = Q->tail = n;
	}
	else{
		Q->tail->next = n;
		Q->tail = n;
	}

	Q->curr_len++;
	n->next = NULL;
}

int pop_head(struct queue *Q){

	int ret;
	ret = Q->head->val;
	while(Q->curr_len == 0){
		pthread_cond_wait(&has_data, &mutex);
	}

	if(Q->head == Q->tail){
		free(Q->head);
		Q->head = Q->tail = NULL;
	}
	else{
		struct node * temp = Q->head;
		Q->head = Q->head->next;
		free(temp);
	}
	Q->curr_len--;
	return ret;
}

void* worker_thread(void* arg){
	
	while(1){

		pthread_mutex_lock(&mutex);
		int newsockfd = pop_head(&waiting);
		pthread_cond_signal(&has_space);
		pthread_mutex_unlock(&mutex);

		char buffer[256];
		bzero(buffer, 256);
		int n = read(newsockfd,buffer, 255);
		if (n < 0)
			error("ERROR reading from socket");

		if((buffer[0] == 'g' || buffer[0] == 'G') && (buffer[1] == 'e' || buffer[1] == 'E') && (buffer[2] == 't' || buffer[2] == 'T')
					&& (buffer[3] == ' ') ){
			// get file from the server and send to client
			
			char filepath[100]; //string copy
			int i;
		
			for(i=4;buffer[i]!='\n';i++){
				filepath[i-4] = buffer[i];
			}
			filepath[i-4] = '\0';

			char sendbuffer[PACKETSIZE];

			FILE* fp;
			fp = fopen(filepath, "rb");
			if(fp == NULL){
				error("Error in opening file\n");
			}

			int bytesSent;
			int read = 0;
			
	        //send file 512 bytes at a time
			while((read = fread(sendbuffer, sizeof(sendbuffer), 1, fp)) > 0){
				bytesSent = write(newsockfd, sendbuffer, PACKETSIZE);

				if(bytesSent < 0)
					error("Error while writing to socket");
			}
			fclose(fp);
		}

		else{
			printf("Invalid Command received\n");
			
			/* send reply to client */
			n = write(newsockfd,"Invalid Command", 15);
			if (n < 0) error("ERROR writing to socket");
		}
		close(newsockfd);
	}

	return 0;
}

int main(int argc, char *argv[]){

	int sockfd, newsockfd, portno, clilen;
	
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 4) {
		fprintf(stderr,"ERROR, usage : ./server-mt <port> <number of worker threads> <queue size>\n");
		exit(1);
	}
	n_threads = atoi(argv[2]);
	waiting.max_len = atoi(argv[3]);

	int i;
	pthread_t threads[n_threads];
	for(i=0;i<n_threads;i++){
		pthread_create(&threads[i], NULL, worker_thread, NULL);
	}

	/* create socket */

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	/* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* bind socket to this port number on this machine */

	if (bind(sockfd, (struct sockaddr *) &serv_addr,
			 sizeof(serv_addr)) < 0) 
			 error("ERROR on binding");
	
	/* listen for incoming connection requests */

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);
	
	/* accept a new request, create a newsockfd */
	while(1){

		pthread_mutex_lock(&mutex);		
		if(waiting.max_len == waiting.curr_len){
			// take care of this later
			// some cond var needed
			// mostly reject
		}
		pthread_mutex_unlock(&mutex);

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			 error("ERROR on accept");

		pthread_mutex_lock(&mutex);
		add(&waiting, newsockfd);
		pthread_cond_signal(&has_data);
		pthread_mutex_unlock(&mutex);
	}

	return 0; 
}