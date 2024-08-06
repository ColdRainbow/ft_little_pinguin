obj-m += main.o 
 
PWD := $(CURDIR) 
 
all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
 
clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod main.ko

uninstall:
	sudo rmmod main

check-style:
	../linux/scripts/checkpatch.pl --file main.c --strict

check:
	cat /proc/mymounts
