#Place the names of all your tracks here (as RRA filenames)

RRA = template.rra
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

all : $(RRA)

%.rra	: %.x
		./$<

%.x	: %.c
		gcc -Wall -g -o $@ -I$(INCLUDE) $< -L$(LIB) -lsong -lm 

mix		: $(RRA)
		rrafastmixer -a0.5 $(RRA) | rplay

play	: $(RRA)
		rplay $(RRA)

.PHONY : clean

clean :
		rm -f $(RRA)
