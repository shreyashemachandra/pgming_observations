#include<stdio.h>

int main(){
	unsigned int i=1024;
	while(1){
		printf("Enter the value of i: \n");
		scanf("%d",&i);

		printf("%d is the power of 2? ---> ",i);
		if((i & (i-1)) == 0){
			printf("Yes\n");
		}else{
			printf("No \n");
		}
	}
	return 0;
}
