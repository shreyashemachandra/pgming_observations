#include<stdio.h>

int main(){
	int x,y;
	x=1;
	y=-1;

	printf("%d , %d  -> (x^y) < 0 \n",(x^y),((x^y) < 0));

	x=1;
	y=1;
	printf("%d , %d  -> (x^y) < 0 \n",(x^y),((x^y) < 0));
	return 0;
}
