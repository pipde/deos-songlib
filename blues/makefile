# Place the names of all your tracks here (as RRA filenames)

RRAS = blues.rra
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

all : $(RRAS)

# comment out the rplay line if you don't want automatic playing

%.rra	: %.x
	./$<

%.x	: %.c
	gcc -Wall -g -o $@ -I$(INCLUDE) $< -L$(LIB) -lsong -lm

play	: $(RRAS)
	rplay $(RRAS)

.PHONY : clean

clean :
	-rm -f $(RRAS)
