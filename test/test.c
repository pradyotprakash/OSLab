#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

int childpg;

void signal_callback_handler_child(int signum){
	printf("Child signal handler called\n");
	exit(0);
}

void signal_callback_handler_parent(int signum){
	printf("Parent signal handler called\n");
	killpg(childpg, signum);
}



int main(){
	int i;
	signal(SIGINT, signal_callback_handler_parent);
	pid_t pid;
	char* argv[2];
	argv[0] = (char*)malloc(2);
	strcpy(argv[0], "ls");
	argv[1] = (char*)NULL;
	if((pid=fork()) == 0){
		setpgid(0,0);
		signal(SIGINT, signal_callback_handler_child);
		printf("%d\n", getpgid(0));
		while(1){
			;
		}
		exit(0);
	}
	childpg = pid;
	printf("%d\n", getpgid(0));
	// printf("%d\n", getpgid(pid));
	wait();
	printf("Something\n");
	free(argv[0]);
	return 0; 
}