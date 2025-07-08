obj-m += mouse_shortcut.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules CC=x86_64-linux-gnu-gcc-13

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
