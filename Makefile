CFLAGS=-DDO_OUTPUT

detector : main.o detector.o
	gcc -g -o detector main.o detector.o `glib-config --libs`

%.o : %.c
	gcc -g $(CFLAGS) -c `glib-config --cflags` $<

clean :
	rm -f *~ *.o detector

dist :
	rm -rf minedetector
	mkdir minedetector
	cp Makefile README COPYING *.[ch] problem* minedetector/
	tar -zcvf minedetector.tar.gz minedetector
	rm -rf minedetector
