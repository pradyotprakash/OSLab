/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/wait.h>

#define PACKETSIZE 512

void error(char *msg){
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]){

	int sockfd, newsockfd, portno, clilen;
	char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
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
		int pid;

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			 error("ERROR on accept");

		if((pid=fork()) == -1){
			close(newsockfd);
			continue;
		}
		else if(pid == 0){ // the child process
			/* read message from client */

			bzero(buffer,256);
			n = read(newsockfd,buffer,255);
			if (n < 0) error("ERROR reading from socket");
			
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
				
				printf("File sent succesfully\n");

				return 0;

			}
			else{
				printf("Invalid Command received\n");
				
				/* send reply to client */
				n = write(newsockfd,"Invalid Command",15);
				if (n < 0) error("ERROR writing to socket");
			}
			close(newsockfd);
		}
		else{ //the parent process
			close(newsockfd);
			int reaped;
			int status;
			while(reaped = waitpid(-1, &status, WNOHANG) > 0){
				if(reaped == pid)
					break;
			}
		}
	}
	return 0; 
}
