TARGET=phrases
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

install		: $(TARGET)
		cp $(TARGET) /usr/local/bin/

$(TARGET)	: $(TARGET).c scanner.c
		gcc -o $(TARGET) -Wall -g -I$(INCLUDE) $(TARGET).c scanner.c -L$(LIB) -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGET)
