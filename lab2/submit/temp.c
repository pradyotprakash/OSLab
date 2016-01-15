#include <stdio.h>
int main(){
	FILE *fp;

	fp = fopen("temp.c", "rb");
	int read;
	char sendbuffer[512];
	read = fread(sendbuffer, sizeof(sendbuffer), 1, fp);
	printf("%s\n", sendbuffer);
	printf("Read: %d\n", read);
	/*
		#include <stdio.h>
int main(){
	FILE *fp;

	fp = fopen("temp.c", "rb");
	int read;
	char sendbuffer[512];
	read = fread(sendbuffer, sizeof(sendbuffer), 1, fp);
	printf("%s\n", sendbuffer);
	printf("Read: %d\n", read);#include <stdio.h>
int main(){
	FILE *fp;

	fp = fopen("temp.c", "rb");
	int read;
	char sendbuffer[512];
	read = fread(sendbuffer, sizeof(sendbuffer), 1, fp);
	printf("%s\n", sendbuffer);
	printf("Read: %d\n", read);	
	*/
}