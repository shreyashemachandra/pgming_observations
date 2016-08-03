#include<stdio.h>
#include<string.h>

int main(){
	char pal[20];
	int i,j;
	printf("Enter the String:\n");
	scanf("%s",pal);
	printf("%s \n",pal);
	i=0;
	j=strlen(pal)-1;

	while(i<j){
		if(pal[i] == pal[j]){
			++i;
			--j;
		}else{
			printf("Not a Palindrome\n");
			return -1;
		}
	}
	printf("%s is a palindrome\n",pal);
	return 0;
}
