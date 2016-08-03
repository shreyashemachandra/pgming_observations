#include <stdio.h>

char str[] = "a3b2f4x6";
char final_str[20];

int main(){
    int size = sizeof(str);
    int i=0;
    int last_index=0;
   
    printf("Char %c int %d",str[1],(str[1]-'0')); 

    for(;i < size;i += 2){
        int j = 0;
        for(;j < str[i+1] - '0'; j++,last_index++)
            final_str[last_index] = str[i];
    }

    printf("Str: %s\n",str);
    printf("Final String : %s \n",final_str);

    return 0;
}

