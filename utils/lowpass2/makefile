TARGETS = lowpass2
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/
base = $(wildcard *.c)
final = $(patsubst %.c,%,$(base))

install		: needed
	cp lowpass2 /usr/local/bin/rralowpass2
	cp lowpass2 /usr/local/bin/rrahighpass2
	cp lowpass2 /usr/local/bin/rrabandpass2
	cp lowpass2 /usr/local/bin/rranotch2
	cp lowpass2 /usr/local/bin/rrapeak2
	cp lowpass2 /usr/local/bin/rralowshelf2
	cp lowpass2 /usr/local/bin/rrahighshelf2

needed : $(final)

%	: %.c
	gcc -o $* -Wall -g -L$(LIB) -I$(INCLUDE) $< -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGETS)
