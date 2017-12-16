TARGETS = chorus
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/
base = $(wildcard *.c)
final = $(patsubst %.c,%,$(base))

install		: needed
	$(foreach var,$(final), cp $(var) /usr/local/bin/rra$(var);)

needed : $(final)

%	: %.c
	gcc -o $* -Wall -g -L$(LIB) -I$(INCLUDE) $< -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGETS)
