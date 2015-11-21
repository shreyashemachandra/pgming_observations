#include<stdio.h>

int main(){
	int x=0x123456;
	int y=0;
	printf("y= %x\n",(x & 0x00FF00));
	return 0;
}
