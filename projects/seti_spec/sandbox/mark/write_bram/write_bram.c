#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

== INPUT ==
arg 1 is the tvg process number 
arg 2 is starting bram address to write
arg 3 is ending bram address to write
arg 4 is bram name
arg 5 is data file of values to be written 

== OUTPUT (stdout) ==
column 1 is the bram address 
column 2 is the value

*/

int main(int argc,char **argv)
{
    if(argv[1] == NULL)
    {
	printf("Usage: write_reg <process number> <start addr> <end addr> <bram name> <data file>\n");
    }
    else
    {
	FILE *bram_file,*data_file;
	char bram[200];
	char file[100];
	char tmp[100];
	memset(file,0,100);
	memcpy(file,argv[5],strlen(argv[5]));
    
	int i;
	int buffer;
     
    	sprintf(bram,"/proc/%s/hw/ioreg/%s",argv[1],argv[4]);
    	sprintf(tmp,"%s",file);
    	bram_file=fopen(bram,"wb");
    	data_file=fopen(file,"r");
   
    	if(bram_file == NULL)
    	{
            printf("Error: couldn't open file '%s'\n",bram); 
            return 1;
    	}
    
        for(i=atoi(argv[2]);i<=atoi(argv[3]);i++)
        {
	    fscanf(data_file,"%s\n",tmp); 
	    buffer=atof(tmp)*32767;
       
            printf("%d %d\n",i,buffer);
            fwrite(&buffer,sizeof(int),1,bram_file);
        }
        
        fclose(bram_file);
	fclose(data_file);
    }
    return 0;
}



