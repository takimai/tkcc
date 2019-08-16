CFLAGS=-std=c11 -g -static

tkcc: tkcc.c

test: tkcc
	./test.sh

clean: 
	rm -f tkcc *.o *~ tmp*

.PHONY: test clean
