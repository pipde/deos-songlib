CC = gcc
CFLAGS = -Wall -g -pg

OBJS = json.o tabproc.o voice.o tempo.o amplitude.o
INCLUDES = json.h tabproc.h voice.h tempo.h amplitude.h

TARGET = libtab.a

$(TARGET) : $(OBJS)
	ar rc $(TARGET) $(OBJS)
	ranlib $(TARGET)

install : $(TARGET)
	mkdir -p /usr/local/lib/
	cp -rf $(TARGET) /usr/local/lib/
	mkdir -p /usr/local/include/tab/
	cp -rf $(INCLUDES) /usr/local/include/tab/

uninstall :
	rm -f /usr/local/lib/$(TARGET)
	for i in $(INCLUDES) ; do rm /usr/local/include/tab/$$i ; done
	rmdir /usr/local/include/tab/
	
json.o : json.c json.h
	$(CC) $(CFLAGS) -c json.c
	
tabproc.o : tabproc.c tabproc.h json.h
	$(CC) $(CFLAGS) -c tabproc.c
	
voice.o : voice.c voice.h
	$(CC) $(CFLAGS) -c voice.c
	
tempo.o : tempo.c tempo.h
	$(CC) $(CFLAGS) -c tempo.c
	
amplitude.o : amplitude.c amplitude.h
	$(CC) $(CFLAGS) -c amplitude.c
		
clean :
	-rm -f $(OBJS) $(TARGET)
