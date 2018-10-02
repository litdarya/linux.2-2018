obj-m += lkm_example.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(shell pwd) clean
restart:
	-sudo rmmod lkm_example
	sudo dmesg -C
	sudo insmod lkm_example.ko
	dmesg
start:
	./helper.sh
