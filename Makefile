obj-m += proc_listing.o
obj-m += run_y_instead_of_x.o
obj-m += input_output.o

KVERSION = $(shell uname -r)

all: xxxxx yyy sys_addresses.h
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean
	rm xxxxx
	rm yyy
	rm sys_addresses.h

xxxxx: x.c
	gcc -o xxxxx x.c

yyy: y.c
	gcc -o yyy y.c

sys_addresses.h:
	./get_addresses.sh > sys_addresses.h

