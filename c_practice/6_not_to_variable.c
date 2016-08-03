#include<stdio.h>

int main(){
	printf("%d  --> !0\n",(!0));
	printf("%d  --> !1\n",(!1));
	printf("%d  --> !(-1)\n",(!(-1)));
	printf("%d  --> !(8)\n",(!(8)));
	printf("%d  --> !(-8)\n",(!(-8)));
	
	printf("%d  --> ~0\n",(~0));
	printf("%d  --> ~1\n",(~1));
	printf("%d  --> ~(-1)\n",(~(-1)));
	printf("%d  --> ~(8)\n",(~(8)));
	printf("%d  --> ~(-8)\n",(~(-8)));
	return 0;
}