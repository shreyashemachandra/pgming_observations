#include<stdio.h>
int stack[5];
int pos=-1;

int isFull(){
	if(pos>4){
		printf("FULLLLLLL\n");
		return 1;
	}else 
		return 0;
}

int isEmpty(){
	if(pos < 0){
		printf("EMPPPPP\n");
		return 1;
	}else
		return 0;
}
int push (int n ){
	printf("!isFull()- %d",!isFull());
	if(!isFull()){
		++pos;
		stack[pos]=n;
		return 1;
	}
	else 
		return 0;
}
int pop(void){
	if(!isEmpty()){
		--pos;
	}
	else
		return -1;
}
void top(){
	printf("TOP: %d\n",stack[pos]);

}
int main (){
	int n,j;
	int x;
	x=-1;
	printf("-1 = %d\n",!x);
	x=0;
	printf("0 = %d\n",!x);
	x=1;
	printf("1 = %d\n",!x);
	x=2;
	printf("2 = %d\n",!x);
	x=200;
	printf("200 = %d\n",!x);
	x=-200;
	printf("-200 = %d\n",!x);

	while(1){
		printf("Enter Your choice\n,1-Push\n2-Pop\n3.isenpty\n4-isFull\n5-Top\n");
		scanf("%d",&n);
		switch(n){
			case 1: scanf("%d",&j);
				printf("Pos %d\n",pos);
				push(j);
				break;
			case 2: pop();
				break;
			case 3: isEmpty();
				break;
			case 4: isFull();
				break;
			case 5: top();
				break;
		}

	}
	return 0;
}
