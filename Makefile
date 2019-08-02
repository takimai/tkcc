tkcc: tkcc.c

test: tkcc
	./test.sh

clean: rm -f tkcc *.o *~ tmp*
