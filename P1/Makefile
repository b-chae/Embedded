CC=arm-none-linux-gnueabi-gcc -pthread -static
CFLAGS=
OBJS=main.o
TARGET=app.out

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)
	
main.o: header.h main.c
