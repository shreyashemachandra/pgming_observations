obj-m := hello_world.o
KERNSRC := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
modules:
	make -C $(KERNSRC) M=$(PWD) modules
clean:
	make -C $(KERNSRC) M=$(PWD) clean