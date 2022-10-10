all: test.o borph.o talker.o 
	gcc -Wall talker.o borph.o -o talker

talker.o: talker.c Makefile
	gcc -c talker.c -o talker.o

borph.o: borph.c Makefile
	gcc -c borph.c -o borph.o

clean:
	@rm -rf *.o

calc: 
	gcc mcalc.c -lm -o mcalc
