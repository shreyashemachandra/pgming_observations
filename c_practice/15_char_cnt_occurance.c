#include <stdio.h>

char str[] = "aaabbcddddmmmsdfffhhhhhhhhhhhhhh";
char temp[10];
int temp_index = 0;

void prepare_temp_arr(){
    int size=sizeof(str);
    int i=0,j;

    for(;i<size;i++){
        for(j=0;j<10;j++){
            if(str[i] == temp[j]){
                break;
            }
        }
        if(j == 10){
            temp[temp_index++]=str[i];
        }
    }
}

void check_occurence(char c){
    int i,count = 0;

    for(i=0;i<sizeof(str);i++){
        if(str[i] == c){
            count++;
        }
    }
    printf("Char %c has %d Occurences\n",c,count);
}

int main(){
    int i = 0;

    prepare_temp_arr();
    for(;i<temp_index;i++){
        check_occurence(temp[i]);
    }
    return 0;
}
