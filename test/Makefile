CC = gcc
CFLAGS = -O3
OFLAGS = -Wall -fopenmp

all: c_test bad

c_test: c_test.c
	$(CC) $(CFLAGS) $(OFLAGS) c_test.c -o c_test.exe

bad: bad.c
	$(CC) $(CFLAGS) $(OFLAGS) bad.c -o bad.exe

clean:
	rm -rf *.exe