INC=
LIB=
DEF=
EXT=
CC=gcc
mkbof : mkbof.c mkbofdef.h
	$(CC) -Wall -I. $(INC) $(LIB) $(DEF) -o mkbof$(EXT) mkbof.c
clean:
	rm -f mkbof mkbof.exe
#mkbofdef.h : idle.strip.ppc
#	od -t u1 -A none -v idle.strip.ppc | sed -e 's/\( \+\)/,\1/g' -e 's/^,//g' -e 's/$/,/g' -e 's/, /,/g' > mkbofdef.h
