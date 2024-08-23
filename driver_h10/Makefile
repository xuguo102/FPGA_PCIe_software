ccflags-y += -DDEBUG
CONFIG_MODULE_SIG=n

obj-m := amd_pcie_exerciser.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	@echo Using built-in clean
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -rf *~
