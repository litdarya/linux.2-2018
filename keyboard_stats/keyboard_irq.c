#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include <asm/switch_to.h>
#include <linux/mutex.h>
#include "keyboard_irq.h"

#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/rwsem.h>


int major = K_MAJOR;
int minor = 0;

struct keyboard_stats_dev *k_dev;

#define MAX_BUFF 10000
DEFINE_SPINLOCK(mr_lock);

void keyboard_tasklet_bh(unsigned long);
DECLARE_TASKLET(keyboard_tasklet, keyboard_tasklet_bh, 0);

/*
 * vector of processed key pressing actions
 * keep it timestamp, use only actuals
 * NOTE: There will be no more then MAX_BUFF last actions encounted
 */
int len = 0;
time_t presses_time[MAX_BUFF];
int end_pointer = 0;

static int num_opened_devices = 0;

void keyboard_tasklet_bh(unsigned long hits) {
  spin_lock(&mr_lock);
  if (!(scancode & 0x80)) {
	  struct timeval t;
	  do_gettimeofday(&t);
	  presses_time[end_pointer++] = t.tv_sec;
	  end_pointer %= MAX_BUFF;
  }
  spin_unlock(&mr_lock);
  return;
}

irq_handler_t irq_handler (int irq, void* dev_id, struct pt_regs* regs) {
  spin_lock(&mr_lock);
  scancode = inb (0x60);
  spin_unlock(&mr_lock);

  tasklet_schedule(&keyboard_tasklet);

  return (irq_handler_t) IRQ_HANDLED;
}

int k_dev_open(struct inode* inode, struct file* file) {
    if (num_opened_devices != 0) {
        return -EBUSY;
    }
    ++num_opened_devices;
    try_module_get(THIS_MODULE);
    return 0;
}

ssize_t k_dev_read(struct file* file, char __user* buf, size_t count, loff_t* f_pos) {
  char tmp[256];
  int i;
  int len_minute;
  struct timeval now;

  if (*f_pos != 0) {
    return 0;
  }

  spin_lock(&mr_lock);
  len_minute = 0;
  do_gettimeofday(&now);

  for (i = 0; i < MAX_BUFF; ++i) {
	  if (now.tv_sec - presses_time[i] < 60) {
		  ++len_minute;
	  }
  }
  sprintf(tmp, "%d\n", len_minute);  
  spin_unlock(&mr_lock);

  if (copy_to_user(buf, tmp, strlen(tmp))) {
    return -EFAULT;
  }

  *f_pos = strlen(tmp);

  return strlen(tmp);
}


int k_dev_release(struct inode* inode, struct file* filp) {
  --num_opened_devices;
  module_put(THIS_MODULE);
  return 0;
}


struct file_operations k_fops = {
  .owner = THIS_MODULE,
  .open  = k_dev_open,
  .read  = k_dev_read,
  .release = k_dev_release,
};


int k_dev_setup(struct keyboard_stats_dev* dev) {
  int err;
  dev_t devno = MKDEV(major, minor);
  
  cdev_init(&dev->cdev, &k_fops);
  dev->cdev.ops = &k_fops;
  dev->cdev.owner = THIS_MODULE;

  err = cdev_add(&dev->cdev, devno, 1);

  if (err) {
    printk(KERN_WARNING "Error during setting up 'keyboard_stats'\n");
    return err;
  }

  memset(presses_time, 0, sizeof(time_t) * MAX_BUFF);

  return 0;
}


static int __init keybrd_int_register(void) {
  int result = -1;
  dev_t dev  = 0;
  
  result = alloc_chrdev_region(&dev, minor, 1, "keyboard_stats");
  if (result < 0) {
    major = MAJOR(dev);
    printk(KERN_WARNING "keyboard_stats: can't get major %d\n", major);
    return result;
  }

  major = MAJOR(dev);

  k_dev = kmalloc(1 * sizeof(struct keyboard_stats_dev), GFP_KERNEL);

  result = k_dev_setup(k_dev);
  if (result)
    return result;

  printk(KERN_INFO "Inserted Module 'keyboard_stats' [%d]\n", major);

  // 1 -- keyboard
  result = request_irq (1, (irq_handler_t) irq_handler, IRQF_SHARED, "keyboard_stats", (void *)(irq_handler));
  if (result)
    printk(KERN_INFO "can't get shared interrupt for keyboard\n");
  
  return result;
}

static void __exit keybrd_int_unregister(void) {
  dev_t devno = MKDEV(major, minor);

  // 1 -- keyboard
  free_irq(1, (void *)(irq_handler));

  cdev_del(&(k_dev->cdev));

  kfree(k_dev);

  unregister_chrdev_region(devno, 1);
  printk(KERN_INFO "Removed Module 'keyboard_stats' [%d]\n", MAJOR(devno));
}

MODULE_LICENSE ("GPL");
module_init(keybrd_int_register);
module_exit(keybrd_int_unregister);
