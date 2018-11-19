obj-m += suprotect.o

all: suprotect-cli
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm suprotect-cli
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

suprotect-cli: suprotect-cli.c
	gcc suprotect-cli.c -o suprotect-cli

