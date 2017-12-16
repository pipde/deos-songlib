TARGET1 = fir
TARGET2 = iir
TARGET3 = hipass
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

install		: $(TARGET1) $(TARGET2) $(TARGET3)
		cp $(TARGET1) /usr/local/bin/rra$(TARGET1)
		cp $(TARGET2) /usr/local/bin/rra$(TARGET2)
		cp $(TARGET3) /usr/local/bin/rra$(TARGET3)

$(TARGET1)	: $(TARGET1).c
		gcc -o $(TARGET1) -Wall -g -I$(INCLUDE) $(TARGET1).c -L$(LIB) -lsong -lm

$(TARGET2)	: $(TARGET2).c
		gcc -o $(TARGET2) -Wall -g -I$(INCLUDE) $(TARGET2).c -L$(LIB) -lsong -lm

$(TARGET3)	: $(TARGET3).c
		gcc -o $(TARGET3) -Wall -g -I$(INCLUDE) $(TARGET3).c -L$(LIB) -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGET1) $(TARGET2) $(TARGET3)
