#include<stdio.h>
#include<string.h>

int bruteForceStringMatching(char *T,int n,char *P,int m){
	int i,j;

	for(i=0;i<=n-m;i++){
		for(j=0;j<m;j++){
			if(T[i+j] != P[j]){
				break;
			}
		}
		if(j == m) return i;
	}
	return -1;
}

int main(){
	char *T="Shreyas @shreyas";
	char *P="@shreyas";
	printf("T: %s \n P: %s\n",T,P);
	int status=bruteForceStringMatching(T,strlen(T),P,strlen(P));
	
	if(status == -1) printf("String Not Foung\n");
	else printf("%s is present in %d in %s\n",P,status,T);	
	return 0;
}
