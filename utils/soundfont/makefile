TARGET = splitter
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

all		: $(TARGET) Scale.class Pitch.class #inst/AddInst.class 

$(TARGET)	: $(TARGET).c
		gcc -o $(TARGET) -Wall -g -I$(INCLUDE) $(TARGET).c -L$(LIB) -lsong -lm
		cp $(TARGET) ../../bin/

Scale.class	: Scale.java
		javac -cp '.:jmusic.jar' Scale.java

Pitch.class	: Pitch.java
		javac -cp '.:jmusic.jar' Pitch.java

inst/AddInst.class	: inst/AddInst.java
			(cd inst; javac *.java)
			
.PHONY: clean

clean:
	-rm -f $(TARGET) inst/*.class *.class
