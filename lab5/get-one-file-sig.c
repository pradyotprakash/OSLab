#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string.h>
#include <signal.h>
 
#define PACKETSIZE 512

int totalBytes; 
void signal_callback_handler(int signum){
    // do something
    printf("Received SIGINT; downloaded %d bytes so far.\n", totalBytes);
    exit(0);
}

void error(char *msg){
    perror(msg);
    exit(0);
}
 
int main(int argc, char *argv[])
{
    // register the signal callback handler
    signal(SIGINT, signal_callback_handler);
    totalBytes = 0;

    int display = 0;
    int sockfd, portno, n;
 
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
    if (argc < 5) {
       fprintf(stderr,"usage %s filename hostname port display/nodisplay\n", argv[0]);
       exit(0);
    }
 
    /* create socket, get sockfd handle */
    char* filename = argv[1];
    if(argv[4][0] == 'd' ){
        display = 1;
    }
 
    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
 
    /* fill in server address in sockaddr_in datastructure */
 
    server = gethostbyname(argv[2]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
 
    /* connect to server */
 
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
 
    /* ask user for input */
    bzero(buffer,256);
    buffer[0] = 'g';
    buffer[1] = 'e';
    buffer[2] = 't';
    buffer[3] = ' ';
 
    int i;
    for(i=0;filename[i]!='\0';i++){
        buffer[4+i] = filename[i];        
    }
 
     
    // int i;
    // for(i=10; buffer[i]!='\0';i++){
    //     filename[i-10] = buffer[i];
    // }
    // filename[i-10] = '\0';
 
    /* send user message to server */
 
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error("ERROR writing to socket");
    bzero(buffer,256);
 
    /* read reply from server */
    char recvbuffer[PACKETSIZE];
    // FILE* fp;
    // fp = fopen(filename, "w");   
    int bytesRead;
    
    while((bytesRead = read(sockfd, recvbuffer, PACKETSIZE)) > 0){
        if(display)
            printf("%s", recvbuffer);
        // fwrite(recvbuffer, 1, bytesRead, fp);
        totalBytes+= bytesRead; 
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
     
    // fclose(fp);
 
    return 0;
}