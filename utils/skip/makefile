base = $(wildcard *.c)
final = $(patsubst %.c,%,$(base))
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

install		: needed
	$(foreach var,$(final), cp $(var) /usr/local/bin/rra$(var);)

needed : $(final)

%	: %.c
	gcc -o $* -Wall -g -I$(INCLUDE) $< -L$(LIB) -lsong -lm

.PHONY: clean

clean:
	-rm -f $(TARGETS)
