TARGET = dcshift
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

install		: $(TARGET)
		cp $(TARGET) /usr/local/bin/rra$(TARGET)

$(TARGET)	: $(TARGET).c
		gcc -o $(TARGET) -Wall -g -I$(INCLUDE) $(TARGET).c -L$(LIB) -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGET)
