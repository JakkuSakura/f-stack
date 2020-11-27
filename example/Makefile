LIBS+= -Wl,--whole-archive,-lfstack,--no-whole-archive
LIBS+= -Wl,--whole-archive,-ldpdk,--no-whole-archive
LIBS+= -Wl,--no-whole-archive -lrt -lm -ldl -lcrypto -lnuma -pthread

OBJS := fstack_daemon.o buffer.o mutex_stack.o

all: main_control libfstack_daemon.a

main_control: libfstack_daemon.a
	cc -O -o main_control main_control.c -L. -lfstack_daemon ${LIBS}

libfstack_daemon.a: $(OBJS)
	ar rcs libfstack_daemon.a ${OBJS}

#libfstack_whole.a:
#	ar rcs libfstack_whole.a ../lib/*.o
#
#libdpdk_whole.a:
#	mkdir -p dpdk && cd dpdk && find ../../dpdk/x86_64-native-linuxapp-gcc/lib/ -name "*.a" -exec ar x {} \;
#	ar rcs libdpdk_whole.a dpdk/*.o

%.o: %.c
	cc -MD -MP -fPIC $< -c -o $@

.PHONY: clean
clean:
	rm -f *.o main_control *.a
	rm -r dpdk

