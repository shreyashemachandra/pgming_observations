#include <signal.h>
#include <stdio.h>
#include <unistd.h>

void ouch(int sig){
	printf("OUCH! − I got signal %d\n", sig);
	signal(SIGINT, SIG_DFL); // SIG_DFL -> Restores the default Behaviour
}

int main(){
	signal(SIGINT, ouch);
	int count=10;
	while(1) {
		count--;
		printf("Hello World!\n");
		if(count==0){
			kill(getpid(),SIGINT);
		}
		sleep(1);
	}
}
