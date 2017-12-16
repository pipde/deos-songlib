ALSALIB = /usr/lib/
INCLUDE=/usr/local/include/songlib/
LIB=/usr/local/lib/

ALL = rra2wav wav2rra rradata rrainfo rrastrip

install	: $(ALL)
		cp $(ALL) /usr/local/bin/

rra2wav	: rra2wav.c
	gcc -o rra2wav -Wall -g -I$(INCLUDE) rra2wav.c -L$(LIB) -lsong -lm

wav2rra	: wav2rra.c
	gcc -o wav2rra -Wall -g -I$(INCLUDE) wav2rra.c -L$(LIB) -lsong -lm

rradata	: rradata.c
	gcc -o rradata -Wall -g -I$(INCLUDE) rradata.c -L$(LIB) -lsong -lm

rrainfo	: rrainfo.c
	gcc -o rrainfo -Wall -g -I$(INCLUDE) rrainfo.c -L$(LIB) -lsong -lm

rrastrip : rrastrip.c
	gcc -o rrastrip -Wall -g -I$(INCLUDE) rrastrip.c -L$(LIB) -lsong -lm

.PHONY: clean

clean:
	-rm -f rrainfo rradata wav2rra rra2wav rrastrip
