TARGET1=hartley
TARGET2=fhartley

all	: $(TARGET1) $(TARGET2)

$(TARGET1)	: $(TARGET1).c
		gcc -o $(TARGET1) -Wall -g $(TARGET1).c -L../../lib -lsong -lm
		cp $(TARGET1) ../../bin/rra$(TARGET1)

$(TARGET2)	: $(TARGET2).c
		gcc -o $(TARGET2) -Wall -g $(TARGET2).c -L../../lib -lsong -lm
		cp $(TARGET2) ../../bin/rra$(TARGET2)

.PHONY: clean

clean:
	-rm -f $(TARGET1) $(TARGET2)
