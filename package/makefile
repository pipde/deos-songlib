ver=`grep "define LIBRARY_VERSION" ../lib/songlib.c | sed "s/.* \"//" | sed "s/\";//"`
arch=i386
#arch=amd64
#arch=OSX
base=songlib-$(ver)-$(arch)
#location=songlib/
location=songlib/trunk/

all : songlib-tar

songlib-tar	:
	rm -f -r ../songlib-export
	#(cd ../lib; sudo make install)
	#(cd ../utils; make)
	svn export .. ../songlib-export/
	rm -r ../songlib-export/examples/
	rm -f ../songlib-export/fairuse/*.mp?
	rm -f ../songlib-export/fairuse/*.wav
	rm -f ../songlib-export/fairuse/*.rra
	rm -f ../songlib-export/utils/soundfont/*.sf2
	(cd ../songlib-export; tar cvfz ../package/songlib.tgz *)
	mv songlib.tgz songlib-$(ver).tgz

install	: songlib-tar
	scp songlib-$(ver).tgz beastie.cs.ua.edu:/var/www/html/songlib/

clean	:
	@rm -f -r ~/songlib-export
	@rm -f -r debian
	@rm -f $(base).deb
	@rm -f rrapack.deb
	@rm -rf rrapack
	@rm -f -r $(base)
	@rm -f $(base).tgz
