#include <stdio.h>
#include <stdlib.h>

/*
== INPUT ==
arg 1 is directory string e.g. '/proc/1234/hw/ioreg/' 

arg 2 is fifo name e.g. 'fifo_name'
== OUTPUT - stdout ==
column 1 is the fifo address and column 2 is the value
*/

int main(int argc,char **argv)
{
    
    if( argv[1] == NULL )
    {
	printf("Usage: read_fifo </proc/directory/> <fifo name>\n");
    }
    else
    {
	FILE *fifo_file;
	char fifo[200]; int i; size_t bytesread;
//	signed int buffer; 
	void *buffer=(void *)malloc(16); 

	sprintf(fifo,"%s/%s",argv[1],argv[2]);
	fifo_file=fopen(fifo,"rb");

	if(fifo_file == NULL)
	{
	    printf("Error: couldn't open file '%s'\n",fifo); 
	    return 1;
	}
    
	while(1)
	{
	    bytesread = fread(&buffer,4*sizeof(int),1,fifo_file);
	    if(bytesread != 0)
	    {
		fread(&buffer,4*sizeof(int),1,fifo_file);
		printf("%d\n",buffer);
	    }
	}
        
	fclose(fifo_file);
    }
    return 0;
}


