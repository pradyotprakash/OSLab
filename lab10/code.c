#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/time.h>

#define SIZE 512
#define NUMFILES 25

int main(int argc, char* argv[]){
	printf("Pid: %d\n", getpid());
	
	int i;

	unsigned long long int n;

	char buf[SIZE];
	char bufs[1];

	struct stat statbuf;
	char* src[25];

	if(argc < 3){
		printf("Usage : ./a.out is_mem_mapped(true/false) read/write\n");
		exit(1);
	}
	// int fd = open("file", O_RDWR);

	// if (fstat (fd, &statbuf) < 0)
	// 	perror("fstat error");

	// scanf("%d", &i);

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
	bzero(buf, SIZE);
	bufs[0] = 0;



	for(i=1;i<NUMFILES;i++){
		sprintf(buf, "file%d", i);
		if((fds[i] = open(buf, O_RDWR)) == -1)
			perror("File open error");

		if(argv[1][0] == 't'){ //if mem mapped
			// printf("here\n");
			if (fstat (fds[i], &statbuf) < 0)
				perror("fstat error");

			int flag = (argv[2][0] == 'r')?PROT_READ:PROT_WRITE;

			if ((src[i] = mmap((caddr_t)0, statbuf.st_size, flag, MAP_SHARED, fds[i], 0)) == (caddr_t) -1)
				perror("mmap error for input");	
		}
		
	}


	gettimeofday(&start, NULL);

	for(i=1;i<=NUMFILES;++i){
		int j;
		if(argv[1][0] == 't'){ // if mmapped
			if(argv[2][0] == 'r'){ //if read operation
				for (j=0;j<10485760;j++){
					// int temp;
					// if((temp = read(fds[i], bufs, 1)) < 0)
					// 	perror("Read error\n");
					
					n += read(fds[i], bufs, 1);
				}
			}
			else{ // if write operation
				for (j=0;j<10485760;j++){
					// int temp;
					// if((temp = write(fds[i], bufs, 1)) < 0)
					// 	perror("Write error\n");
					
					n += write(fds[i], bufs, 1);
				}
			}
		}
		else{
			if(argv[2][0] == 'r'){
				for(j=0;j<10485760;j+=512){
					// int temp;
					// if((temp = read(fds[i], buf, 512)) < 0)
					// 	perror("Read error\n");
					
					n += read(fds[i], buf, 512);
				}
			}
			else{
				for (j=0;j<10485760;j+=512){
					// int temp;
					// if((temp = write(fds[i], buf, 512)) < 0)
					// 	perror("Write error\n");
					
					n += write(fds[i], buf, 512);

				}
			}
		}
		
		close(fds[i]);
	}
	gettimeofday(&end, NULL);
	double totTime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
	printf("Average: %f MB/s\n", (n/totTime)/1000000.0);
}