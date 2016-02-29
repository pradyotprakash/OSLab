#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#define N 10
#define K 10000

int count;
int locked;
void* inc(){
	int i;
	for (i = 0; i < K; ++i){
		while(locked);
		locked = 1;
		count++;
		locked = 0;
	}
}

int main(){
	int i;
	pthread_t threads[N];
	locked = 0;
	struct timeval start, end;

	gettimeofday(&start, NULL);

	for(i=0;i<N;i++ ){
		pthread_create(&threads[i], NULL, inc, NULL);
	} 

	for(i=0;i<N;i++ ){
		pthread_join(threads[i], NULL);
	}

	gettimeofday(&end, NULL);

	double total_time = end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec)/1000000.0;

	printf("count : %d\n", count);
	printf("Time : %f\n", total_time);
}