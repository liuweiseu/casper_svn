all: test.o testlib.o
	gcc test.o testlib.o -o test

test.o: test.c
	gcc -c test.c -o test.o


testlib.o: testlib.c
	gcc -c testlib.c -o testlib.o

clean: 
	@rm *.o
