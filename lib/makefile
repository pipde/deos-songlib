CC = gcc
DEBUG = -DMIN_DEBUG_LEVEL=3 -DMAX_DEBUG_LEVEL=6 -g #-p
CFLAGS = -Wall -g #-pg -Wall $(DEBUG)

OBJS = debug.o songlib.o util.o wave.o rra.o key.o drums.o guitar.o hartley.o filters.o
INCLUDES = debug.h key.h rra.h songlib.h util.h wave.h drums.h guitar.h hartley.h

TARGET = libsong.a

$(TARGET) : $(OBJS)
	ar rc $(TARGET) $(OBJS)
	ranlib $(TARGET)

install: $(TARGET)
	mkdir -p /usr/local/lib
	cp -rf libsong.a /usr/local/lib/
	mkdir -p /usr/local/include/songlib/
	cp -rf *.h /usr/local/include/songlib/

uninstall:
	./libuninstall

debug.o		: debug.c debug.h
		$(CC) $(CFLAGS) -c debug.c

drums.o	: drums.c drums.h songlib.h debug.h
		$(CC) $(CFLAGS) -c drums.c

guitar.o	: guitar.c guitar.h songlib.h debug.h
		$(CC) $(CFLAGS) -c guitar.c

key.o		: key.c key.h songlib.h
		$(CC) $(CFLAGS) -c key.c

rra.o		: rra.c rra.h util.h
		$(CC) $(CFLAGS) -c rra.c

songlib.o	: songlib.c songlib.h key.h drums.h rra.h debug.h
		$(CC) $(CFLAGS) -c songlib.c

wave.o		: wave.c wave.h util.h debug.h
		$(CC) $(CFLAGS) -c wave.c

util.o		: util.c util.h
		$(CC) $(CFLAGS) -c util.c

hartley.o	: hartley.c hartley.h rra.h
		$(CC) $(CFLAGS) -c hartley.c

filters.o	: filters.c filters.h
		$(CC) $(CFLAGS) -c filters.c

testlib : testlib.c
		sudo make install
		$(CC) $(CFLAGS) testlib.c -lsong -lm
		a.out
		rplay testlib.rra


clean :
	-rm -f $(OBJS) $(TARGET)
