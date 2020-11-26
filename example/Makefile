LIBS+= -Wl,--whole-archive,-lfstack,--no-whole-archive
LIBS+= -Wl,--whole-archive,-ldpdk,--no-whole-archive
LIBS+= -Wl,--no-whole-archive -lrt -lm -ldl -lcrypto -pthread -lnuma

OBJS := fstack_daemon.o buffer.o mutex_stack.o

all: main_control libfstack_daemon.a

main_control: libfstack_daemon.a
	cc -O -o main_control main_control.c ${LIBS} -lfstack_daemon -L.

libfstack_daemon.a: $(OBJS)
	ar rcs libfstack_daemon.a ${OBJS}

%.o: %.c
	cc -MD -MP -fPIC $< -c -o $@

.PHONY: clean
clean:
	rm -f *.o main_control *.a

