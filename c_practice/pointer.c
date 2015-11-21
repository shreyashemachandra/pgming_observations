#include<stdio.h>

int a[5]={1,2,3,4,5};
int b[5]={6,7,8,9,10};

void swap(int **p){
    *p=b;
}

int main(){
    int *p,i=0;
    p=a;
    printf("before:\n");
    for(;i<5;i++)   
        printf("p[%d] -> %d",i,p[i]);

    swap(&p);

    printf("\nAfter: \n");
    for(i=0;i<5;i++)
        printf("p[%d] -> %d",i,*(p+i));

    printf("\n");

    return 0;
}
