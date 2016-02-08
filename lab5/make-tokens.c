#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

struct _ret{
	char** t;
	int l;
};

void signal_callback_handler_child(int signum){
	printf("\n");
	exit(0);
}

void signal_callback_handler_parent(int signum){
	printf("Hello>");
	// exit(0);
}

struct _ret tokenize(char *line){
	struct _ret r;
	r.t = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i=0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				r.t[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(r.t[tokenNo++], token);
				tokenIndex = 0; 
			}
		}
		else {
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	r.t[tokenNo] = NULL ;
	r.l = tokenNo;
	return r;
}


void main(void){

	char  line[MAX_INPUT_SIZE];
	struct _ret ret;
	char  **tokens;
	int i, l;

	char* server_ip = NULL;
	char* server_port = NULL;

	pid_t bpgid = -1;

	while (1) {
		char path[1000];
		getcwd(path, 1000);
		printf("Hello::%s>", path);
		bzero(line, MAX_INPUT_SIZE);
		gets(line);
		line[strlen(line)] = '\n'; //terminate with new line
		ret = tokenize(line);
		tokens = ret.t;
		l = ret.l;

		if(l == 0)
			continue;

		char temp[64];
		sprintf(temp, "/bin/%s", tokens[0]);
		
		//check if that command is a std bash executable
		if(access(temp, F_OK) != -1){
			pid_t pid = fork();
			if(pid == 0){
				char* argv[64];
				int i;
				for( i=0; i < l; i++ ){
					argv[i] = tokens[i];
					// printf("%s\n", argv[i]);
				}
				argv[i] = (char*)NULL;
				int retCode = execvp(temp, argv);
				
				if(retCode == -1){
					fprintf(stderr, "Error while running %s! Error code: %d\n", tokens[0], errno);
				}

				exit(0);
			}
			// signal(SIGINT, signal_callback_handler_parent);
			setpgid(pid, pid);
			wait();
			
		}
		else if(strcmp(tokens[0], "cd") == 0){

			if(l != 2){
				fprintf(stderr, "Usage cd: cd <path>\n");
				continue;
			}

			int ret = chdir(tokens[1]);
			
			if (ENOTDIR == ret){
				fprintf(stderr, "cd %s: Path does not exist!\n", tokens[1]);
			}
			else if(ret == 0){
			}
			else{
				fprintf(stderr, "cd %s: Some error occurred!\n", tokens[1]);	
			}
		}
		else if(strcmp(tokens[0], "server") == 0){
			
			if(l != 3){
				fprintf(stderr, "Usage server: server <server-ip> <server-port>\n");
				continue;
			}

			if(strlen(tokens[1]) > 15 || strlen(tokens[2]) > 5){
				fprintf(stderr, "server: Invalid arguments!\n");
				continue;
			}

			if(server_ip == NULL || server_port == NULL){
				server_ip = (char*) malloc(sizeof(char) * strlen(tokens[1]));
				server_port = (char*) malloc(sizeof(char) * strlen(tokens[2]));
			}

			strcpy(server_ip, tokens[1]);
			strcpy(server_port, tokens[2]);

			printf("Server details stored!\n");
		}
		else if(strcmp(tokens[0], "getfl") == 0){
			
			if(server_port == NULL || server_ip == NULL){
				fprintf(stderr, "Run 'server' command first!\n");
				continue;
			}

			if(l == 2){

				pid_t pid = fork();
				if(pid == 0){
					
					char arg[1000];
					sprintf(arg, "%s %s %s display", tokens[1], server_ip, server_port);
					
					int retCode = execlp("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL);
					
					if(retCode == -1){
						fprintf(stderr, "Error while running getfl! Error : %s\n", strerror(errno));
					}

					exit(0);
				}
				setpgid(pid, pid);
				// signal(SIGINT, signal_callback_handler_parent);
				wait();
			}

			else if(l == 4 && strcmp(tokens[2], ">") == 0){
				
				pid_t pid = fork();
				
				if(pid == 0){
					
					char arg[1000];
					sprintf(arg, "%s %s %s display", tokens[1], server_ip, server_port);
					
					close(1);
					int fd = open(tokens[3], O_RDWR | O_CREAT);
					dup2(fd, 1);

					int retCode = execlp("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL);
					
					if(retCode == -1){
						fprintf(stderr, "Error while running getfl! Error : %s\n", strerror(errno));
					}

					close(fd);

					exit(0);
				}
				setpgid(pid, pid);
				wait();
			}
			else if(l >= 4 && strcmp(tokens[2], "|") == 0){
								
				
				int pipefd[2];

				if (pipe(pipefd) == -1) {
					perror("pipe");
					continue;
				}
				pid_t pid, pid1;

				if((pid=fork()) == 0){
					// left side child process
					close(1);
					dup(pipefd[1]);
					close(pipefd[0]);
					close(pipefd[1]);
					char arg[1000];
					sprintf(arg, "%s %s %s display", tokens[1], server_ip, server_port);
					
					int retCode = execlp("./get-one-file-sig", "./get-one-file-sig", tokens[1], server_ip, server_port, "display", NULL);
					
					if(retCode == -1){
						fprintf(stderr, "Error while running getfl! Error : %s\n", strerror(errno));
					}

					exit(0);
				}
				pid1 = pid;
				setpgid(pid, pid); //NOTE: setting the pgid of both children to the same group

				if((pid=fork()) == 0){
					close(0);
					dup(pipefd[0]);
					close(pipefd[0]);
					close(pipefd[1]);
					char temp1[64];
					sprintf(temp1, "/bin/%s", tokens[3]);
					if(access(temp, F_OK) != -1){
						char* argv[64];
						int i;
						for( i=0; i < l; i++ ){
							argv[i] = tokens[i+3];
							// printf("%s\n", argv[i]);
						}
						argv[i] = (char*)NULL;
						int retCode = execvp(temp, argv);
						
						if(retCode == -1){
							fprintf(stderr, "Error while running %s! Error : %s\n", tokens[0], strerror(errno));
						}

						exit(0);
					}
				}	
				setpgid(pid, pid1);

				close(pipefd[0]);
				close(pipefd[1]);
				wait();
				wait();

			}
			else{
				fprintf(stderr, "Usage getfl: getfl <filename>\n");
				fprintf(stderr, "Usage getfl: getfl <filename> | <command>\n");
				fprintf(stderr, "Usage getfl: getfl <filename> > <output file>\n");
				continue;
				
			}
		}
		else if(strcmp(tokens[0], "getsq") == 0){
			if(l < 2){
				fprintf(stderr, "Usage getsq: getsq <filename_1> .. <filename_n>\n");
				continue;
			}
			else{
				for(i=1;i<l;++i){
					pid_t pid = fork();
					
					if(pid == 0){
									
						char arg[1000];
						sprintf(arg, "%s %s %s display", tokens[i], server_ip, server_port);
						
						int retCode = execlp("./get-one-file-sig", "./get-one-file-sig", tokens[i], server_ip, server_port, "nodisplay", NULL);
						
						if(retCode == -1){
							fprintf(stderr, "Error while running getfl! Error : %s\n", strerror(errno));
						}
						exit(0);
					}
					setpgid(pid, pid);
					wait();
				}
			}
		}
		else if(strcmp(tokens[0], "getpl") == 0){
			if(l < 2){
				fprintf(stderr, "Usage getpl: getpl <filename_1> .. <filename_n>\n");
				continue;
			}
			else{
				pid_t p_top;
				if((p_top=fork())==0){
					pid_t pgid = -1;
					for(i=1;i<l;++i){
						pid_t pid = fork(); 
						if(pid == 0){
										
							char arg[1000];
							sprintf(arg, "%s %s %s display", tokens[i], server_ip, server_port);
							
							int retCode = execlp("./get-one-file-sig", "./get-one-file-sig", tokens[i], server_ip, server_port, "nodisplay", NULL);
							
							if(retCode == -1){
								fprintf(stderr, "Error while running getfl! Error : %s\n", strerror(errno));
							}
							exit(0);
						}
						if(pgid==-1)
							pgid = pid;
						setpgid(pid, pgid);
					}
					while(waitpid(-1, NULL, WNOHANG) > 0);
				}
				setpgid(p_top, p_top);
				wait();
			}
		}
		else if(strcmp(tokens[0], "getbg") == 0){
			if(l != 2){
				fprintf(stderr, "Usage getbg: getbg <filename> \n");
				continue;
			}
			else{
				pid_t pid = fork();
				
				if(pid == 0){	
					char arg[1000];
					sprintf(arg, "%s %s %s display", tokens[i], server_ip, server_port);
					
					int retCode = execlp("./get-one-file-sig", "./get-one-file-sig", tokens[i], server_ip, server_port, "nodisplay", NULL);
					
					if(retCode == -1){
						fprintf(stderr, "Error while running getfl! Error : %s\n", strerror(errno));
					}
					exit(0);
				}
				setpgid(pid, pid); //TODO: see if this is to be done
			}
		}
		else if(strcmp(tokens[0], "exit") == 0){
			if(l!=1){
				fprintf(stderr, "Usage exit: exit \n");
				continue;	
			}
			//kill all background processes
			exit(0);
		}
		else{
			printf("FOund some!\n");
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		
		free(tokens);
	}

	free(ret.t);
	free(server_port);
	free(server_ip);
}