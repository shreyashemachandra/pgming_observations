#include<stdio.h>
#include<unistd.h>

int main(int argc,char * args[]){
		pid_t pid;
		pid=fork();
		if(pid == -1){
			printf("Process not Created\n");
			return -1;
		}
		if(pid > 0){
			int chaild_status;
			printf("Parent Process Address Space\n");
			waitpid(pid,&chaild_status,0);
			printf("Child Exited -- Now Parent Exitting\n");
		}else if(pid == 0){
			printf("Child Process Address Space\n");
			execv("./hello",NULL);
		}
		return 0;
}