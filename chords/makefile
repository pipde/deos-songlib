# Place the names of all your tracks here (as RRA filenames)

RRAS = chords.rra 
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

all : $(RRAS)

# comment out the rplay line if you don't want automatic playing

%.rra	: %.x
	./$<
	#rplay $@

%.x	: %.c
	gcc -Wall -g -o $@ -I$(INCLUDE)  $< -L$(LIB) -lsong -lm

b.rra	: drums.rra
	cat drums.rra | rrastereo -a1 -r > b.rra

a.rra	: chords.rra
	cat chords.rra | rrastereo -a1 -l > a.rra

song.rra : a.rra
	rrafastmixer a.rra > song.rra #b.rra > song.rra

play	: song.rra
	rplay song.rra

.PHONY : clean

clean :
	-rm -f $(RRAS)
