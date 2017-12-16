TARGETS = chorus2
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/
base = $(wildcard *.c)
final = $(patsubst %.c,%,$(base))

install		: needed
	$(foreach var,$(final), cp $(var) /usr/local/bin/rra$(var))
	cp chorus2 /usr/local/bin/rraflanger2
	cp chorus2 /usr/local/bin/rravibrato2

needed : $(final)

%	: %.c
	gcc -o $* -Wall -g -L$(LIB) -I$(INCLUDE) $< -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGETS)
