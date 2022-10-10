#include <stdio.h>
#include <stdlib.h>

//INPUT
//argument 1 is the tvg process number 
//
//argument 2 is starting bram address to read
//argument 3 is ending bram address to read 
//argument 4 is bram name
//
//OUTPUT - stdout
//column 1 is the bram address and column 2 is the value

int format2comp(unsigned char u_val)
{
        int s_int, i = 0;
            unsigned int s_max = 127; //Max positive signed value
                unsigned int mask = 0xFFFFFFFF ^ s_max;
                    if(u_val > s_max)
                                s_int = (int) mask | u_val;
                        else
                                    s_int = (int) u_val;
                            
                            return s_int;
}  


int main(int argc,char **argv)
{
    if(argv[1] == NULL)
    {
        printf("Usage: read_bram <process number> <start addr> <end addr> <bram name>\n");
    }
    else
    {
	FILE *bram_file;
	char bram[200];
	int i,j;
	unsigned char buffer;
 
	sprintf(bram,"/proc/%s/hw/ioreg/%s",argv[1],argv[4]);
	bram_file=fopen(bram,"rb");

    if(bram_file == NULL)
    {
        printf("Error: couldn't open file '%s'\n",bram); 
        return 1;
    }
    
    for(i=atoi(argv[2]);i<=atoi(argv[3]);i++)
    {
        printf("%d,\t", i);
        for(j=0;j<4;j++){
        fread(&buffer,sizeof(char),1,bram_file);

//These lines convert 'buffer' to signed value            
//            if(buffer > 131072)
//                buffer = (int) 0xFFFC0000 | buffer;
            
         printf("%d,\t",format2comp(buffer));
        }
        printf("\n");    
    }
        
    fclose(bram_file);
    } 
    return 0;
}


