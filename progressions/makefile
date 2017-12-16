# Place the names of all your tracks here (as RRA filenames)

RRAS = ionian.rra lydian.rra mixolydian.rra \
       aeolian.rra phrygian.rra dorian.rra locrian.rra \
       melodic_minor.rra harmonic_minor.rra \
       strange.rra prog.rra
INST = /usr/local/share/samples/guitar-electric/clean_a1.rra
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

single : $(INST) prog.rra

all : $(INST) $(RRAS)

# comment out the rplay line if you don't want automatic playing

%.rra	: %.x 
	./$<
	rplay $@

%.x	: %.c
	gcc -Wall -g -o $@ -I$(INCLUDE) $< -L$(LIB) -lsong -lm

$(INST)	:
		mkdir temp
		(cd temp; getpack guitar-electric-clean)
		rm -r temp

mixed.rra	: $(RRAS)
	rramix $(RRAS) mixed.rra

mix	: mixed.rra

play	: mixed.rra
	rplay mixed.rra

.PHONY : clean

clean :
	-rm -f $(RRAS)
