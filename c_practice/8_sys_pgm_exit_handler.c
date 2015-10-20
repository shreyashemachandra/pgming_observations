#include<stdio.h>
#include<stdlib.h>

void exit_handler(void){
	printf("Exit Handler Called!!, Programming Exitting\n");
}

void fun1(){
	printf("Fun1 Called!\n");
	exit(EXIT_SUCCESS);
}

int main(int argc,char * args[]){
		printf("Inside Main\n");
		atexit(exit_handler);
		fun1();
		printf("Main Exiting!\n");
		return 0;
}
