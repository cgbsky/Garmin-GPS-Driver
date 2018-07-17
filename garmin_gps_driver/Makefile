obj-m += garmin_gps.o
	CURRENT_PATH:=$(shell pwd)
	LINUX_KERNEL_PATH:=/lib/modules/$(shell uname -r)/build
all:
	$(MAKE) -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko *.tmp_versrions *.order *.symvers
	rm -rf .tmp_versions
