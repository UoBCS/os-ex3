KERNELDIR=/lib/modules/`uname -r`/build
#ARCH=i386
#KERNELDIR=/usr/src/kernels/`uname -r`-i686

EXTRA_CFLAGS += -I$(PWD)
MODULES = charDeviceDriver.ko 
obj-m += charDeviceDriver.o 
PROGS = myioctl myread mywrite


all: $(MODULES) $(PROGS)

charDeviceDriver.ko: charDeviceDriver.c
	make -C $(KERNELDIR) M=$(PWD) modules

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -f $(PROGS) *.o *.o.rc


myioctl: myioctl.c
	gcc $(CFLAGS) -o $@ $<

myread: myread.c
	gcc $(CFLAGS) -o $@ $<

mywrite: mywrite.c
	gcc $(CFLAGS) -o $@ $<
