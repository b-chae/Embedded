CC=arm-none-linux-gnueabi-gcc -pthread -static
CFLAGS=
OBJS=main.o input.o output.o
TARGET=app.out

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS)
	
main.o: header.h main.c
input.o: header.h input.c
output.o: header.h output.c