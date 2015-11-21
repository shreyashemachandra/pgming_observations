#include<stdio.h>

int main(){
	int arr[10];
	int i=0;
	printf("Enter 10 Elements:\n");
	for(;i<10;i++){
		scanf("%d",&arr[i]);
	}

	int smallestIndex=0;
	
	for(i=0;i<10;i++){
		if(i !=smallestIndex && arr[i]<arr[smallestIndex] ){
			smallestIndex=i;
		}
	}
	
//	printf("Smallest Index: %d\n",smallestIndex);
	int secondSmallest=arr[0];
	for(i=0;i<10;i++){
		if(i != smallestIndex && arr[i] < secondSmallest){
			secondSmallest=arr[i];
		}
	}
	printf("Second Smallest: %d\n",secondSmallest);
	
	
	return 0;
}
