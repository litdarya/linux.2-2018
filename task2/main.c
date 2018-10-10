#include "/home/darya/kernelbuild/linux-4.18.6/customsyscall/person.h"
#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


int main()
{  
    struct Person* test = (struct Person*)malloc(sizeof(struct Person));
    memset(test, 0, sizeof(test));
    strcpy(test->surname, "2");
//    long int ret_status = syscall(335, NULL, (unsigned int)1, test);
    long int ret_status = syscall(335, (long)1);
         
    if(ret_status == 0) 
         printf("Use dmesg to check processInfo\n");
    
    else 
         printf("error syscall %d\n", ret_status);
          
     return 0;
}
