#include<stdio.h>
#include<limits.h>

union myUnion{
	char q,w,e,r,t,y,u,i,o;
	double a;
};

int main(){
	
	printf("size of Int: %d\n size of double: %d\n sizeof char: %d\n",sizeof(int),sizeof(double),sizeof(char));
	printf("size of Union: %d\n",sizeof(union myUnion));
//	printf("Word Size: %d",WORD_BIT);
	
	return 0;
}
