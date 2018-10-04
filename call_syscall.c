#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

struct Person {
    char name[MAX_NAME_LEN];
    char surname[MAX_NAME_LEN];
    char phone[MAX_NAME_LEN];
    char email[MAX_NAME_LEN];

};

int main()
{  
    struct Person* test = (struct Person*)malloc(sizeof(struct Person));
     
    long int ret_status = syscall(335, "1", 1, test);
         
    if(ret_status == 0) 
         printf("System call 'listProcessInfo' executed correctly. Use dmesg to check processInfo\n");
    
    else 
         printf("System call 'listProcessInfo' did not execute as expected\n");
          
     return 0;
}
