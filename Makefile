CFLAGS=-std=c11 -g -static -fno-common
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)


tkcc: $(OBJS)
	$(CC) -g -o $@ $(OBJS) $(LDFLAGS) 
$(OBJS): tkcc.h

test: tkcc
	./test.sh

clean: 
	rm -f tkcc *.o *~ tmp*

.PHONY: test clean
