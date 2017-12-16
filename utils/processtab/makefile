CC = gcc
CFLAGS = -Wall -g
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/
TARGET = processtab
OBJS = processtab.o tabparser.o tabchecker.o

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(CFLAGS) $(OBJS) -L$(LIB) -lsong

install : $(TARGET)
	mkdir -p /usr/local/bin/
	cp $(TARGET) /usr/local/bin/
	
uninstall :
	rm /usr/local/bin/$(TARGET) 

processtab.o : processtab.c tabchecker.h
	$(CC) $(CFLAGS) -c -I$(INCLUDE) processtab.c

tabparser.o : tabparser.c tabparser.h
	$(CC) $(CFLAGS) -c -I$(INCLUDE) tabparser.c
		
tabchecker.o : tabchecker.c tabchecker.h tabparser.h
	$(CC) $(CFLAGS) -c -I$(INCLUDE) tabchecker.c
	
clean :
	-rm -f $(OBJS) $(TARGET) $(BIN)

