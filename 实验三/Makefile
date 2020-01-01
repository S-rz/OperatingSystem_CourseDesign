ifneq ($(KERNELRELEASE),)
obj-m := MyDev.o
else
	PWD := $(shell pwd)
	KERNEL_VER ?= $(shell uname -r)
	KERNEL_DIR := /lib/modules/$(KERNEL_VER)/build
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c

endif
