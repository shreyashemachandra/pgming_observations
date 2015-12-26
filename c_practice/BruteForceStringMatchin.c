#include<stdio.h>
#include<string.h>

int bruteForceStringMatching(char *T,int n,char *P,int m){
	int i;
	for(i=0;i<=n-m;i++){ // Loop Till n-m
		int j=0;
		while(j<m && T[i+j]==P[j])
			++j;

		if(j==m)
			return i;	
	}
	return -1;
}

int main(){
	// T - Actual text to search for String, with Size n
	// P - Pattern to be matched in T, with Size m
	char *T="Shreyas #@shre Name my Bad";
	char *P=" Name";
	int n,m;
	int status=0;

	//printf("Enter Text T:\n");
	//scanf("%[\^n]",T);
	printf("T is: %s \n\n",T);
	//printf("Enter the SubString P:\n");
	//scanf("%[\^n]",P);
	printf("P is: %s\n\n",P);
	
	n=strlen(T);
	m=strlen(P);
	printf("Length of T: %d\n Length of P: %d\n",n,m);
	status=bruteForceStringMatching(T,n,P,m);
	if(status!=-1)
		printf("The Pattern(P) starts from %d index in Text(T)!\n",status);
	else
		printf("Pattern P is not found!\n");
	
	return status;
}
