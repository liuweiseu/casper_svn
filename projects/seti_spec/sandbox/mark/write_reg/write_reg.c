#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//writes value to software registers 
//
//INPUT
//arg 1 is process number
//arg 2 is register
//arg 3 is register value
//declarations

int main(int argc, char **argv){


    if( argv[1] == NULL)
    {
        printf("Usage: write_reg <process number> <register name> <integer value>\n");
    }
    else
    {

	int register_value=atoi(argv[3]);
	char reg_file_string[100];
     
	FILE *register_file;

	//write value to register file
    
	sprintf(reg_file_string,"/proc/%s/hw/ioreg/%s",argv[1],argv[2]);
	register_file=fopen(reg_file_string,"wb");      

	if(register_file == NULL)
	{
	    printf("Error: could not open register file %s\n",reg_file_string);
	    return 1;
	}

	fwrite(&register_value,4,1,register_file);
	fclose(register_file);  
    }
	return 0;

}
