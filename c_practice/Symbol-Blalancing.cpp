#include<iostream>
#include<stdio.h>
#include<stack>

using namespace std;

int main(){
	char *equation;
	equation="(a+b-v})*(a+b)";
	int i=0;
	stack<char> sym;
	
	while(equation[i] != '\0'){
		char c=equation[i];
		if((c!='{' & c!='[' & c!='}' & c!=']' & c!='(' & c!=')') == 0){
			
			if( (c=='{' | c=='[' | c=='(') == 1){
				sym.push(c);	
			}else{
				if((c==')' && sym.top()=='(') || (c=='}' && sym.top()=='{') || (c==']' && sym.top()=='[')){
					sym.pop();
				}	
			}	
		}
		i++;
	}
	printf("\n");
	if(sym.empty()){
		printf("Equation is Valid\n");
	}else{
		printf("Equation is Not Valid\n");
	}
	
	return 0;
}
