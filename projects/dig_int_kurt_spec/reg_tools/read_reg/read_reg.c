#include <stdio.h>
#include <stdlib.h>

//INPUT
//argument 1 is the tvg process number 
//
//argument 2 is starting reg address to read
//argument 3 is ending reg address to read 
//argument 4 is reg name
//
//OUTPUT - stdout
//column 1 is the reg address and column 2 is the value

int main(int argc,char **argv)
{
    if(argv[1] == NULL)
    {
        printf("Usage: read_reg <process number> <reg name>\n");
    }
    else
    {
	FILE *reg_file;
	char reg[200];
	unsigned int buffer;
    int i;
	sprintf(reg,"/proc/%s/hw/ioreg/%s",argv[1],argv[2]);
	reg_file=fopen(reg,"rb");

    	if(reg_file == NULL)
    	{
            printf("Error: couldn't open file '%s'\n",reg); 
            return 1;
    	}
    
    	fread(&buffer,sizeof(int),1,reg_file);

//These lines convert 'buffer' to signed value            
//            if(buffer > 131072)
//                buffer = (int) 0xFFFC0000 | buffer;
            
        //printf("%d\n",buffer);
        
        printf("0x%08X / 0b", buffer);
        for(i=31;i>=0;i--) {
            printf("%d", (buffer>>i)&1);
         }
        printf(" / %010d\n\r", buffer);
 
        fclose(reg_file);

    } 

    return 0;
}


