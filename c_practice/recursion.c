#include<stdio.h>

int reverse(int i){
	if(i==0){
		return 0;
	}else{
		printf("%d\t",i);
	}
	reverse(--i);
}

int main(){
	int no=5;
	reverse(no);
	return 0;
}
