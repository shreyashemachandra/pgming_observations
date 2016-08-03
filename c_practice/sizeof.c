#include<stdio.h>

#define my_sizeof(type) (char *)(&type+1) - (char *)(&type)

int main(){
	int *arr;
	arr=(int *)malloc(20*sizeof(int));
	printf("%d\n",sizeof(arr));
//	printf("%p - %p = %d\n",arr,arr+1,(char *) (arr+1)- (char *)arr);
	return 0;
}
