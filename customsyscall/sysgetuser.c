#define _GNU_SOURCE

#include "sysgetuser.h"

#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/syscalls.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#include<linux/sched.h>

#define MAX_NAME_LEN 30
#define MAX_COMMAND_LEN 100
#define RESULT_SIZE (MAX_NAME_LEN * 5 + 10)

int write_call( int fd, const char* str, int len ) {
   long __res;
   __asm__ volatile ( "int $0x80":
      "=a" (__res):"0"(__NR_write),"b"((long)(fd)),"c"((long)(str)),"d"((long)(len)) );
   return (int) __res;
}

int read_call( int fd, const char* str, int len ) {
   long __res;
   __asm__ volatile ( "int $0x80":
      "=a" (__res):"0"(__NR_read),"b"((long)(fd)),"c"((long)(str)),"d"((long)(len)) );
   return (int) __res;
}

int open_call( const char* path, int flags ) {
   long __res;
   __asm__ volatile ( "int $0x80":
      "=a" (__res):"0"(__NR_open),"b"((long)(path)),"c"((long)(flags)) );
   return (int) __res;
}

int close_call( int fd ) {
   long __res;
   __asm__ volatile ( "int $0x80":
      "=a" (__res):"0"(__NR_close),"b"((long)(fd)) );
   return (int) __res;
}

char* get_query(const char* command, const char* surname) {
    char* res = kmalloc(strlen(command) + strlen(surname) + 3, GFP_KERNEL);
    if (sprintf(res, "%s %s\n", command, surname) < 0) {
        res[0] = '\0';
        printk(KERN_ALERT "Unable to get query sprintf\n");
    }
    return res;
}

int get_next_arg(char* argument, const char* buffer, size_t len) {
    int i = 0;
    for (i = 0; i < len && i < MAX_NAME_LEN - 1; ++i) {
        if (buffer[i] == ' ' || buffer[i] == '\n' || buffer[i] == '\0') {
            strncpy(argument, buffer, i);
            argument[i] = '\0';
              
            if (buffer[i] == '\0') {
                return i;
            }
            return i + 1;
        }
    }
    return i;
}

void string_to_person(const char * query, struct Person* person) {
    int offset_buf;
    offset_buf = 0;
    int i = 0;
    printk(KERN_DEBUG "string_to_person\n");
    char all_args[5][MAX_NAME_LEN];
    char devnull[MAX_NAME_LEN];
    memset(person, 0, sizeof(struct Person));
    for (i = 0; i < 5; ++i) {
        offset_buf += get_next_arg(devnull, query + offset_buf, strlen(query) - offset_buf);
        offset_buf += get_next_arg(all_args[i], query + offset_buf, strlen(query) - offset_buf);
    }
    
    strcpy(person->name, all_args[0]);
    strcpy(person->surname, all_args[1]);
    strcpy(person->phone, all_args[2]);
    strcpy(person->email, all_args[3]);
    strcpy(person->age, all_args[4]);
}

//asmlinkage long sys_nothing(void) {
//    printk(KERN_DEBUG "nothing\n");
//    return 0;
//}

//asmlinkage long sys_sysgetuser(const char* surname, unsigned int len, struct Person* user_data) {
asmlinkage long sys_sysgetuser(long len) {
     printk(KERN_DEBUG "sys_sysgetuser");
     printk(KERN_DEBUG "len= %ld\n", len);
     /*char* query;
     char answer[RESULT_SIZE];
     int fd = open_call("/dev/lkm_example", O_RDWR);
     if (fd < 0) {
         printk(KERN_ALERT "Can not open device /dev/lkm_example\n");
         return -1;
     }

     //query = get_query("get", surname);
     query = "get 1";
     write_call(fd, query, strlen(query));
     read_call(fd, answer, RESULT_SIZE);
     printk(KERN_DEBUG "Answer:%s \n", answer);
     close_call(fd);
     kfree(query);*/
     return 0;
}

asmlinkage long sys_put_user(struct Person* user_data) { 
     return 0;
}

asmlinkage long sys_del_user(const char* surname, unsigned int len, struct Person* user_data) {
     return 0;
}

