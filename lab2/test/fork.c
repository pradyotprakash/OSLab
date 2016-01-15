#include <stdio.h>
#include <unistd.h>

int main() {
	int x = fork();
	if(x == 0)
		printf("From child: Pid: %d\n", x);
	else printf("From parent: Pid: %d\n", x);
	
	char *const a1, a2;

	int y = execve("./temp", a1, a2);
	printf("Execve: %d\n", y);
}