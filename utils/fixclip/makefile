TARGET1 = fixclip
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

install		: $(TARGET1) 
		cp $(TARGET1) /usr/local/bin/rra$(TARGET1)

$(TARGET1)	: $(TARGET1).c
		gcc -o $(TARGET1) -Wall -g -I$(INCLUDE) $(TARGET1).c -L$(LIB) -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGET1)
