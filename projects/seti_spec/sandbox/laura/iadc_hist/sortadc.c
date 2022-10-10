#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int i = 0;
	unsigned int inadc[8192];
	unsigned char inname[150];
	unsigned int inread[0];
	FILE *infile;

	sprintf(inname, "%s", argv[1]);
	infile=open(inname, "r");

	for(i=0; i<2048; i++)
	{	
//		fread(&inread, 4*sizeof(char), 1, infile);
		read(infile, inread, 4);
		printf("%u\n", inread[0]);
		inadc[4*i] = inread[0] & 0x000000FF;
		inadc[4*i+1] = ((inread[0] & 0x0000FF00) >> 8);
		inadc[4*i+2] = ((inread[0] & 0x00FF0000) >> 16);
		inadc[4*i+3] = ((inread[0] & 0xFF000000) >> 24); 
	}
	
	for(i=0; i<8192; i++)
		printf("%u \n", inadc[i]);

	fclose(infile);
	return 0;
}
