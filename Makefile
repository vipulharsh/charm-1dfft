CHARMC=../charm/bin/charmc $(OPTS) -O3 -g

all: fft

fft:  fft.o
	$(CHARMC) fft.o -o fft -language charm++

fft.o : fft.C fft.ci complex.h
	$(CHARMC) -c fft.ci
	$(CHARMC) -c fft.C

clean:
	rm -f *.def.h *.decl.h *.o *~ charmrun fft
