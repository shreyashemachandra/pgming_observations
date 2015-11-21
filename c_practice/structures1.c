#include<stdio.h>

struct temp{
	int i;
	char a;
	short int g;
	int x;
	int t;
};

int main(){
	struct temp ins;
	printf("Size of : %d\n",sizeof(short int));	
	printf("size of %d\n",sizeof(struct temp));
	
	return 0;
}
