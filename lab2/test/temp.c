#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <string>
using namespace std;

int main(){
	string c = gethostbyname("localhost");
	printf("%s\n", c);
}