TARGETS = reverb2
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/
base = $(wildcard *.c)
final = $(patsubst %.c,%,$(base))

install		: needed
	cp reverb2 /usr/local/bin/rrareverb2

needed : $(final)

%	: %.c
	gcc -o $* -Wall -g -L$(LIB) -I$(INCLUDE) $< -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGETS)
