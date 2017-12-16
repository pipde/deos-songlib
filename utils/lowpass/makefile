TARGETS = lowpass
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/
base = $(wildcard *.c)
final = $(patsubst %.c,%,$(base))

install		: needed
	cp lowpass /usr/local/bin/rralowpass
	cp lowpass /usr/local/bin/rrahighpass

needed : $(final)

%	: %.c
	gcc -o $* -Wall -g -L$(LIB) -I$(INCLUDE) $< -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGETS)
