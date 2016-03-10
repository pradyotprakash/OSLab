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
// mutex to lock the queue
pthread_mutex_t mutex;
// conditional varibales to check if the queue is empty and has space
pthread_cond_t has_data, has_space;

void error(char *msg){
	perror(msg);
	exit(1);
}

// a queue node
struct node{
	int val;
	struct node* next;
};

// the queue to store the fds
struct queue{
	struct node *head, *tail;
	int max_len;
	int curr_len;
} waiting;

// routine to add an element to the end of the queue
void add(struct queue *Q, int fd){

	struct node * n = (struct node*) malloc(sizeof(struct node));
	n->val = fd;
	// while(Q->curr_len == Q->max_len && Q->max_len!=0){
	// 	pthread_cond_wait(&has_space, &mutex);
	// }

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

// routine to remove the head of the queue and return that
int pop_head(struct queue *Q){

	int ret;
	if(Q->head == NULL){
		return -1;
	}
	ret = Q->head->val;
	// while(Q->curr_len == 0){
	// 	pthread_cond_wait(&has_data, &mutex);
	// }

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

// routine to print the queue
void print(struct queue* Q) {
	if(Q->head==NULL){
		printf("Empty queue\n");
		return;
	}
	struct node* ptr = Q->head;
	while(ptr != NULL){
		printf("%d ->", ptr->val);
		ptr = ptr->next;
	}
	printf("\n");
}

// function defining the working of the worked thread
void* worker_thread(void* arg){
	
	while(1){
		// if queue is empty, then wait for it to contain something
		pthread_mutex_lock(&mutex);
		while(waiting.curr_len == 0){
			printf("Queue empty\n");
			pthread_cond_wait(&has_data, &mutex);
		}

		// extract the first element from the queue
		int newsockfd = pop_head(&waiting);
		
		// signal that queue has space since the first element has been popped
		pthread_cond_signal(&has_space);
		pthread_mutex_unlock(&mutex);

		char buffer[256];
		bzero(buffer, 256);
		int n = read(newsockfd,buffer, 255);
		if (n < 0){
			printf("ERROR reading from socket");
			continue;
		}

		// server code from the previous labs
		if((buffer[0] == 'g' || buffer[0] == 'G') && (buffer[1] == 'e' || buffer[1] == 'E') && (buffer[2] == 't' || buffer[2] == 'T')
					&& (buffer[3] == ' ') ){
			// get file from the server and send to client			
			char filepath[100]; //string copy
			int i;
		
			for(i=4;buffer[i]!='\0';i++){
				filepath[i-4] = buffer[i];
				// printf("%c\n", buffer[i]);
			}
			filepath[i-4] = '\0';

			char sendbuffer[PACKETSIZE];

			FILE* fp;
			fp = fopen(filepath, "rb");
			if(fp == NULL){
				printf("Error in opening file\n");
				continue;
			}

			int bytesSent;
			int read = 0;
			
	        //send file 512 bytes at a time
			while((read = fread(sendbuffer, sizeof(sendbuffer), 1, fp)) > 0){
				bytesSent = write(newsockfd, sendbuffer, PACKETSIZE);

				if(bytesSent < 0){
					printf("Error while writing to socket\n");
					break;	
				}
			}
			fclose(fp);
		}

		else{
			printf("Invalid Command received\n");
			
			/* send reply to client */
			n = write(newsockfd,"Invalid Command", 15);
			if (n < 0) {
				printf("ERROR writing to socket\n");
				continue;
			}
		}
		close(newsockfd);
	}

	return 0;
}

void sigpipe_handler(int signum){
	worker_thread(NULL);
}

int main(int argc, char *argv[]){
	signal(SIGPIPE, sigpipe_handler);

	int sockfd, newsockfd, portno, clilen;
	// initialize the head and tail of the queue to NULL to indicate an empty queue
	waiting.head = waiting.tail = NULL;
	
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 4) {
		fprintf(stderr,"ERROR, usage : ./server-mt <port> <number of worker threads> <queue size>\n");
		exit(1);
	}
	n_threads = atoi(argv[2]);

	// max length of the queue
	waiting.max_len = atoi(argv[3]);

	int i;
	pthread_t threads[n_threads];
	// create the necessary number of threads
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

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			 error("ERROR on accept");

		pthread_mutex_lock(&mutex);
		
		// if no space in the queue, then wait for space to become available
		// this condition checked only if the queue is bounded
		while(waiting.max_len == waiting.curr_len && waiting.max_len != 0){
			printf("Queue full\n");
			print(&waiting);
			pthread_cond_wait(&has_space, &mutex);
		}
		
		// add the socket file descriptor to the queue
		add(&waiting, newsockfd);
		// signal that data is available
		pthread_cond_signal(&has_data);
		pthread_mutex_unlock(&mutex);
	}

	return 0; 
}