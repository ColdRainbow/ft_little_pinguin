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

perms:
	sudo chmod 777 /sys/kernel/debug
	sudo chmod +r /sys/kernel/debug/fortytwo

check-debugfs:
	sudo stat /sys/kernel/debug/fortytwo

read-id:
	cat /sys/kernel/debug/fortytwo/id

write-id-wrong:
	echo "bla" > /sys/kernel/debug/fortytwo/id

write-id-right:
	echo -n "jkate" > /sys/kernel/debug/fortytwo/id

read-jiffies:
	cat /sys/kernel/debug/fortytwo/jiffies

foo:
	echo "Hello, debugfs!" > /sys/kernel/debug/fortytwo/foo
	cat /sys/kernel/debug/fortytwo/foo

check-style:
	../linux/scripts/checkpatch.pl --file main.c
