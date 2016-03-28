#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/time.h>

#define SIZE 512
#define FILESIZE 10485760
#define NUMFILES 25
#define READSIZE FILESIZE
#define WRITESIZE FILESIZE
#define U 2

void expt1(){
	struct stat statbuf;
	int fd = open("file", O_RDWR), i;
	char buf[SIZE];
	char *src;

	if (fstat (fd, &statbuf) < 0)
		perror("fstat error");

	printf("File size: %d\n", statbuf.st_size);

	scanf("%d", &i);

	if ((src = mmap ((caddr_t)0, statbuf.st_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == (caddr_t) -1)
		perror("mmap error for input");

	scanf("%d", &i);

	bzero(buf, SIZE);
	char c = src[0];

	scanf("%d", &i);

	printf("Size: %d\n", sizeof(src));
	src[100000] = 'A';

	scanf("%d", &i);
	msync(src[0], FILESIZE, MS_SYNC);
	munmap(src[0], FILESIZE);
	close(fd);
}

void expt2(int argc, char* argv[]){
	struct timeval start, end;
	struct stat statbuf;
	char buf[SIZE];
	char *src[NUMFILES];
	int fds[NUMFILES], i;
	unsigned long long int n=0;
	
	gettimeofday(&start, NULL);
		
	for(i=0;i<NUMFILES;i++){
		bzero(buf, SIZE);
		sprintf(buf, "file%d", i);
		if((fds[i] = open(buf, O_RDWR)) == -1)
			perror("File open error");

		if(argv[1][0] == 't'){ //if mem mapped
			if (fstat (fds[i], &statbuf) < 0)
				perror("fstat error");

			int flag = (argv[2][0] == 'r')?PROT_READ:PROT_WRITE;

			if ((src[i] = mmap((caddr_t)0, statbuf.st_size, flag, MAP_SHARED, fds[i], 0)) == (caddr_t) -1)
				perror("mmap error for input");	
		}
	}


	for(i=0;i<NUMFILES;++i){
		unsigned int j;
		if(argv[1][0] == 't'){ // if mmapped
			if(argv[2][0] == 'r'){ //if read operation
				char c;
				for(j=0;j<READSIZE;++j)
					c = src[i][j];
				n += READSIZE;
			}
			else{ // if write operation
				for(j=0;j<WRITESIZE;++j)
					src[i][j] = 'a';
				n += WRITESIZE;
			}
			msync(src[i], FILESIZE, MS_SYNC);
			munmap(src[i], FILESIZE);
		}
		else{
			if(argv[2][0] == 'r'){
				for(j=0;j<READSIZE;j+=SIZE){					
					n += read(fds[i], buf, SIZE);
				}
			}
			else{
				// memset(buf, c, SIZE);
				for (j=0;j<WRITESIZE;j+=SIZE){
					n += write(fds[i], buf, SIZE);
				}
			}
		}
		close(fds[i]);
	}
	gettimeofday(&end, NULL);
	double totTime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
	printf("Average: %f MB/s\n", (n/totTime)/(1024*1024));
}

void expt3(){
	struct timeval start, end;
	struct stat statbuf;
	char buf[SIZE];
	char *src[U];
	int fds[U], i;
	unsigned long long int n=0;
		
	for(i=0;i<U;i++){
		sprintf(buf, "file%d", i);
		if((fds[i] = open(buf, O_RDWR)) == -1)
			perror("File open error");

		if (fstat (fds[i], &statbuf) < 0)
			perror("fstat error");

		if ((src[i] = mmap((caddr_t)0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fds[i], 0)) == (caddr_t) -1)
			perror("mmap error for input");	
	}

	char c;
	int j;
	for(i=0;i<U;++i){
		for(j=0;j<READSIZE;j+=SIZE){					
			n += read(fds[i], buf, SIZE);
		}
	}

	n = 0;

	gettimeofday(&start, NULL);
	for(i=0;i<U;++i){
		for(j=0;j<WRITESIZE;++j)
			src[i][j] = (char)j;
		n += WRITESIZE;

		msync(src[i], FILESIZE, MS_SYNC);
		munmap(src[i], FILESIZE);
		// close(fds[i]);
	}

	gettimeofday(&end, NULL);

	double totTime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
	printf("Average: %f MB/s\n", (n/totTime)/(1024*1024));

	n = 0;
	gettimeofday(&start, NULL);
	for(i=0;i<U;++i){
		for(j=0;j<WRITESIZE;j+=SIZE)
			n += write(fds[i], buf, SIZE);
		
		close(fds[i]);
	}

	gettimeofday(&end, NULL);

	totTime = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;
	printf("Average: %f MB/s\n", (n/totTime)/(1024*1024));	
}

int main(int argc, char* argv[]){
	printf("Pid: %d\n", getpid());
	
	if(argc < 4){
		printf("Usage : ./a.out is_mem_mapped(true/false) read/write expt_no.\n");
		exit(1);
	}

	if(atoi(argv[3]) == 1)
		expt1();
	else if(atoi(argv[3]) == 2)
		expt2(argc, argv);
	else if(atoi(argv[3]) == 3)
		expt3();
	else
		fprintf(stderr, "Invalid argument\n");
}