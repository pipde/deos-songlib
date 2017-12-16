ALL :  $(patsubst %.tex,%.html,$(wildcard *.tex)) modes.rra

%.rra	: %.x
	./$<

%.x	: %.c
	gcc $(LOCAL) -Wall -g -o $@ $< -lsong -lm 

play	: modes.rra
	rplay modes.rra

%.html : %.tex
	pdflatex $<
	hyperlatex $<
	fix $@

install: $(ALL)
	scp lusth.css *.html *.tex *.pdf beastie.cs.ua.edu:/var/www/songlib/

local: $(ALL)
	cp *.html *.tex *.pdf /var/www/songlib/

.PHONY : clean

clean : 
	-rm -f *.html *.pdf *.log
	-rm -f /var/www/songlib/*.html /var/www/songlib/*.tex /var/www/songlib/*.pdf
