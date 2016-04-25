CHARMC=../charm/bin/charmc $(OPTS) -O3 -g

all: fft transfft

fft:  fft.o
	$(CHARMC) fft.o -o fft -language charm++

fft.o : fft.C fft.ci complex.h
	$(CHARMC) -c fft.ci
	$(CHARMC) -c fft.C

transfft:  transfft.o
	$(CHARMC) transfft.o -o transfft -language charm++

transfft.o : transfft.C transfft.ci complex.h
	$(CHARMC) -c transfft.ci
	$(CHARMC) -c transfft.C

clean:
	rm -f *.def.h *.decl.h *.o *~ charmrun fft transfft
