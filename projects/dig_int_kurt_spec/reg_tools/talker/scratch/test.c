#include <stdio.h>
#include "borph.h"

int main()
{
    int value0;
    int value1;
    int value2;
    int value3;
    int value4;
    int value5;
    char *value6;

    FILE *fd;
    char config_info_buf[200];

    char fft_proc[50] = "23258";
    char pfb_proc[50] = "23270";
    char thr_proc[50] = "23256";
    
    value0 = read_addr(fft_proc,"fft_shift",fd);
    value1 = read_addr(pfb_proc,"fft_shift",fd);
    value2 = read_addr(thr_proc,"thr_comp1_thr_lim",fd);
    value3 = read_addr(thr_proc,"thr_scale_p1_scale",fd);
    value4 = read_addr(thr_proc,"rec_reg_10GbE_destport0",fd);
    value5 = read_addr(thr_proc,"rec_reg_ip",fd);
    value6 = timeo();

    sprintf(config_info_buf,"BEE TIME: %s\nPFB SHIFT: %d\nFFT SHIFT: %d\nTHRESH LIMIT: %d\nTHRESH SCALE: %d\nTENGE PORT: %d\nTENGEIP: %d\n",value6,value0,value1,value2,value3,value4,value5);

    printf("%s",config_info_buf);

    return 0;
}
