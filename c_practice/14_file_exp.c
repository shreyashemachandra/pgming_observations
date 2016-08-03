#include<stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

unsigned int system_rev;
static int get_system_rev(void)
{
        FILE *fp;
        char buf[1024];
        int nread;
        char *tmp, *rev;
        int ret = -1;

        fp = fopen("/proc/cpuinfo", "r");
        if (fp == NULL) {
                perror("/proc/cpuinfo\n");
                return ret;
        }
	printf("open Success!\n");
        nread = fread(buf, 1, sizeof(buf), fp);
	printf(" nread : %d \n",nread);
        fclose(fp);
 //       if ((nread == 0) || (nread == sizeof(buf))) {
        if ((nread == 0)) {
             //   fclose(fp);
		printf("Return\n");
                return ret;
        }
        buf[nread] = '\0';

        tmp = strstr(buf, "Revision");
	printf("tmp = %s\n",tmp);
        if (tmp != NULL) {
                rev = index(tmp, ':');
                if (rev != NULL) {
                        rev++;
                        system_rev = strtoul(rev, NULL, 16);
                        ret = 0;
                }
        }

        return ret;
}

int main(){
	int ret =-1;
	ret = get_system_rev();
        if (ret == -1) {
                printf("Error: Unable to obtain system rev information\n");
                return -1;
        }else{
		printf("Success system_rev: %u\n",system_rev);
	}
	return 0;
}
