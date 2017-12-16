# Place the names of all your tracks here (as RRA filenames)

RRAS = riff.rra
INST = /usr/local/share/samples/guitar-electric/distorted01_1c.rra
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

all : $(INST) $(RRAS)

# comment out the rplay line if you don't want automatic playing

%.rra	: %.x
	./$<

%.x	: %.c
	gcc -Wall -g -o $@ -I$(INCLUDE) $< -L$(LIB) -lsong -lm

$(INST)	:
		mkdir temp
		(cd temp; getpack guitar-electric-distorted01)
		rm -r temp

play	: $(RRAS)
	rplay $(RRAS)

.PHONY : clean

clean :
	-rm -f $(RRAS)
