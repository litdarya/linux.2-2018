obj-m += keyboard_irq.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
restart:
	-sudo rmmod keyboard_irq
	sudo dmesg -C
	sudo insmod keyboard_irq.ko
	dmesg
start:
	sudo insmod keyboard_irq.ko
	dmesg
