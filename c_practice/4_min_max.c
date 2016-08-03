#include<stdio.h>

int main(){
	int x,y;
	x=10;
	y=-10;
	printf("X=10, Y=-10, Min is: %d \n",(y^((x^y) & -(x<y))));
	printf("X=10 Y=-10, Max is: %d \n",(x^((x^y) & -(x<y))));
	return 0;
}
