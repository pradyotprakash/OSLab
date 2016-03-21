#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/time.h>

#define SIZE 512
#define NUMFILES 25

int main(){
	printf("Pid: %d\n", getpid());
	
	int i;

	int n;

	char buf[SIZE];

	// struct stat statbuf;
	// int fd = open("file", O_RDWR);

	// if (fstat (fd, &statbuf) < 0)
	// 	perror("fstat error");

	// scanf("%d", &i);

	// char* src;
	// if ((src = mmap ((caddr_t)0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0)) == (caddr_t) -1)
	// 	perror("mmap error for input");

	// scanf("%d", &i);

	// bzero(buf, SIZE);
	// n = read(fd, buf, 1);
	// // printf("%c %d\n", buf[0], n);

	// scanf("%d", &i);

	// // for(i=0;i<50000;++i){
	// 	lseek(fd, 9999, SEEK_SET);
	// 	bzero(buf, SIZE);
	// 	n = read(fd, buf, 1);
	// 	// printf("%c %d\n", buf[0], n);
	// // }

	// scanf("%d", &i);
	// close(fd);
	///////////////////////////////////////////////////////////////////////
	// Part B
	////////////////////////////////////////////////////////////////////////

	struct timeval start, end;
	int fds[NUMFILES];
	n=0;
	char bufs[1];
	gettimeofday(&start, NULL);
	for(i=1;i<=NUMFILES;++i){
		sprintf(buf, "file%d", i);
		fds[i] = open(buf, O_RDWR);
		int j;
		for (j=0;j<10485760;j++)
			n += read(fds[i], bufs, 1);
		close(fds[i]);
	}
	gettimeofday(&end, NULL);
	double totTime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
	printf("Average: %f\n", (n/totTime));
}