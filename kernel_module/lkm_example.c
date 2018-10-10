#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/string.h>

MODULE_AUTHOR("Litvintseva Darya");
MODULE_DESCRIPTION("Task 1, Linux.2 course");
MODULE_LICENSE("GPL");

#define DEVICE_NAME "lkm_example"
#define MSG_BUFFER_LEN 15
#define EXAMPLE_MSG "Hello, World!\n"

#define MAX_COMMAND_LEN 100
#define RESULT_SIZE (MAX_NAME_LEN * 5 + 10)

#define MAX_NAME_LEN 30

struct Person {
    char name[MAX_NAME_LEN];
    char surname[MAX_NAME_LEN];
    char phone[MAX_NAME_LEN];
    char email[MAX_NAME_LEN];

    char age[MAX_NAME_LEN];
    struct list_head list;
};


static int device_num;
static int num_opened_devices = 0;
static char msg_buffer[MSG_BUFFER_LEN];
static char *msg_ptr;

static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);


static struct file_operations file_functions = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

struct Person adress_book;


char result[RESULT_SIZE] = "empty result";

void flush_result(void) {
     result[0] = '\0';
}

static ssize_t device_read(struct file* flip, char* buffer, size_t len, loff_t* offset) {
    size_t res_len = strlen(result);
    printk(KERN_DEBUG "strlen = %zu; len=%zu\n", res_len, len);
    if (len < res_len) {
        res_len = len;
    }
    copy_to_user(buffer, result, res_len);
    result[0] = '\0';
    return res_len; 
}

void get_all(void) {
    struct Person* iterator;
    char* ptr = result;
    int current_offset = 0;
    size_t bytes_left = RESULT_SIZE;
    printk(KERN_DEBUG "get_all\n");
    memset(result, 0, RESULT_SIZE);
    snprintf(result, bytes_left, "no entries found\n");
    list_for_each_entry(iterator, &adress_book.list, list) {
	current_offset = snprintf(ptr, bytes_left,
            "Name: '%s' Surname: '%s' Phone: '%s' Email: '%s' Age: '%s' \n",
            iterator->name, iterator->surname, iterator->phone, iterator->email, iterator->age);
        if (current_offset < 0) {
            printk(KERN_ALERT "get_all: snprintf failed with offset=%d", current_offset);
        } else {
            ptr += current_offset;
            bytes_left -= current_offset;
        }
    }
    return;
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

ssize_t put_person(const char* data, size_t len) {
    int offset_buf;
    struct Person* new_person;
    offset_buf = 0;
    printk(KERN_DEBUG "put_person\n");
    new_person = kmalloc(sizeof(struct Person), GFP_KERNEL);
    memset(new_person, 0, sizeof(struct Person));
    printk(KERN_DEBUG "offset = %d", offset_buf);
    offset_buf += get_next_arg(new_person->name, data, len);
    printk(KERN_DEBUG "offset = %d; %s", offset_buf, new_person->name);
    offset_buf += get_next_arg(new_person->surname, data + offset_buf, len - offset_buf);
    printk(KERN_DEBUG "offset = %d %s", offset_buf, new_person->surname);
    offset_buf += get_next_arg(new_person->phone, data + offset_buf, len - offset_buf);
    printk(KERN_DEBUG "offset = %d", offset_buf);
    offset_buf += get_next_arg(new_person->email, data + offset_buf, len - offset_buf);
    printk(KERN_DEBUG "offset = %d", offset_buf);
    offset_buf += get_next_arg(new_person->age, data + offset_buf, len - offset_buf);
    printk(KERN_DEBUG "offset = %d", offset_buf);
    INIT_LIST_HEAD(&new_person->list);
    list_add_tail(&(new_person->list), &(adress_book.list));

    printk(KERN_INFO "Added user %s successfully...\n", new_person->name);
    return offset_buf;
}

void get_info(const char* data) {
    struct Person* iterator;
    char* ptr = result;
    int current_offset = 0;
    size_t bytes_left = RESULT_SIZE;
    char surname[MAX_NAME_LEN];
    
    get_next_arg(surname, data, MAX_NAME_LEN);
    
    printk(KERN_DEBUG "get_info\n");
    memset(result, 0, RESULT_SIZE);
    snprintf(result, bytes_left, "no person with surname = '%s' found\n", surname);
    list_for_each_entry(iterator, &adress_book.list, list) {
        if (strcmp(iterator->surname, surname)) {
            continue;
        }
	current_offset = snprintf(ptr, bytes_left,
            "Name: '%s' Surname: '%s' Phone: '%s' Email: '%s' Age: '%s' \n",
            iterator->name, iterator->surname, iterator->phone, iterator->email, iterator->age);
        if (current_offset < 0) {
            printk(KERN_ALERT "get_info: snprintf failed with offset=%d", current_offset);
            flush_result();
        } else {
            ptr += current_offset;
            bytes_left -= current_offset;
        }
    }
    return;
}

void del_info(const char* data) {
    struct Person* iterator;
    struct Person* temp_iterator;
    char surname[MAX_NAME_LEN];
    char* ptr = result;   
    int current_offset = 0;
    size_t bytes_left = RESULT_SIZE;
    
    get_next_arg(surname, data, MAX_NAME_LEN);

    printk(KERN_DEBUG "del_info\n");
    memset(result, 0, RESULT_SIZE);
    snprintf(result, RESULT_SIZE, "no person with surname '%s' found\n", surname);
    
    list_for_each_entry_safe(iterator, temp_iterator, &adress_book.list, list) {
        if (strcmp(iterator->surname, surname)) {
            continue;
        }
            
        current_offset = snprintf(ptr, bytes_left,
            "DELETED Name: %s Surname: %s Phone: %s Email: %s Age: %s \n",
            iterator->name, iterator->surname, iterator->phone, iterator->email, iterator->age);
        if (current_offset < 0) {
            printk(KERN_ALERT "get_all: snprintf failed with offset=%d", current_offset);
        } else {
            ptr += current_offset;
            bytes_left -= current_offset;
        }

        list_del(&iterator->list);
        kfree(iterator);
    }

}

static ssize_t device_write(struct file* flip, const char* buffer, size_t len, loff_t* offset) {
    char command[MAX_COMMAND_LEN + 1];
    memset(command, 0, MAX_COMMAND_LEN + 1);
    strncpy(command, buffer, len);
    printk(KERN_DEBUG "len = %zu", len);
    flush_result();
    if (len == 0) {
        printk(KERN_DEBUG "len == 0");
        return len;
    }
    if (strncmp(command, "put ", 4) == 0) {
        put_person(command + 4, strlen(command + 4));
    } else if (strncmp(command, "get_all", 7) == 0) {
        get_all();
    } else if (strncmp(command, "get ", 4) == 0) {
        get_info(command + 4);
    } else if (strncmp(command, "del ", 4) == 0) {
        del_info(command + 4);
    } else {
        printk(KERN_ALERT "No such command '%s'", command);
    }
    return len;
}


static int device_open(struct inode *inode, struct file *file) {
    if (num_opened_devices != 0) {
        return -EBUSY;
    }
    ++num_opened_devices;
    try_module_get(THIS_MODULE);
    return 0;
}

void clean_module(void) {
    struct Person* iterator;
    struct Person* tmp;
    printk(KERN_INFO "kernel module unloaded \n");
    
    list_for_each_entry_safe(iterator, tmp, &adress_book.list, list){
         printk(KERN_DEBUG "free node %s \n", iterator->name);
         list_del(&iterator->list);
         kfree(iterator);
    }
    return;
}

static int device_release(struct inode *inode, struct file *file) {
    --num_opened_devices;
    module_put(THIS_MODULE);
    return 0;
}

static int __init lkm_example_init(void) {
    strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);
    msg_ptr = msg_buffer;

    device_num = register_chrdev(0, "lkm_example", &file_functions);
    
    if (device_num < 0) {
        printk(KERN_ALERT "Could not register device: %d\n", device_num);
        return device_num;
    }

    INIT_LIST_HEAD(&adress_book.list);

    printk(KERN_INFO "lkm_example module loaded with device number %d\n", device_num);
    return 0;
}

static void __exit lkm_example_exit(void) {
    clean_module();
    unregister_chrdev(device_num, DEVICE_NAME);
    printk(KERN_INFO "Linux.2 task1 finish...\n");
}

module_init(lkm_example_init);
module_exit(lkm_example_exit);

