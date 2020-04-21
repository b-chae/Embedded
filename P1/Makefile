CC=arm-none-linux-gnueabi-gcc
CFLAGS=-static -pthread
OBJS=main.o
TARGET=app.out

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)
	
main.o: header.h main.c