#Makefile for a basic kernel mo
all: app
#all: driver 

app:
	arm-none-linux-gnueabi-gcc -static -o app intertest.c

install:
	cp -a app /nfsroot

clean:
	rm -rf app
