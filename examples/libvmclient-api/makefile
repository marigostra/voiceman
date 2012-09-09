# NOTE: vmclient.h and libvmclient.a should be available in your system.

all: example1 example2 example3

clean:
	rm -f example1 example2 example3 *~ *.o

example1: example1.c
	gcc -oexample1 example1.c -lvmclient

example2: example2.c
	gcc -oexample2 example2.c -lvmclient

example3: example3.c
	gcc -oexample3 example3.c -lvmclient

