#include <stdlib.h>
#include <stdio.h>
main()
{
    int n; 
    FILE *outfile;
    if ((outfile = fopen("ssedata.raw", "wb")) == NULL)
    {
	printf("could not open file ssedata for writing\n");
	exit(1);
    }

    for (n=0; n<128*1024; n++) 
        putc(n, outfile); 
}
