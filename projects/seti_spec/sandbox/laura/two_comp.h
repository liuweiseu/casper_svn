/******************************/
/* Two's compliment functions */
/* 02 Dec 08                  */
/******************************/

#include <math.h>

int format2comp(unsigned int u_val, int bitwidth)
{
	int s_int, i = 0;
	unsigned int s_max = pow(2, bitwidth - 1) - 1; //Max positive signed value
	unsigned int mask = 0xFFFFFFFF ^ s_max;
	if(u_val > s_max)
		s_int = (int) mask | u_val;
	else
		s_int = (int) u_val;
	
	return s_int;
}  

int calc_bw(unsigned int buffer[][2], int bram_len)
{
    unsigned int max = 2; 
    unsigned int int_pow, sbit = 0;
    int i = 0;
/*Find max value (will be negative)*/	
    for(i = 0; i < bram_len; i++)
    {
	    if(buffer[i][0] > max)
		    max = buffer[i][0];
    }
    i = 31;
    while(sbit == 0)
    {
        int_pow = (1 << i);
        if((max & int_pow) == int_pow)
            sbit = i+1;   //Convert index to width
        else
            i--;
    }
	
    return sbit;    //Returns bitwidth, not index
} 
