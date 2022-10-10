#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//writes values to software registers thr_cpu_ctrl and bram_wr_reg 
//thr_cpu_ctrl will specify the length, skip, and stall to the FPGA
//bram_wr_reg will specify the data to be written to the specified 
//address to the specified BRAM (real or imaginary) 
//
//INPUT
//arg 1 is process number
//arg 2,3,4 is length,skip,stall
//arg 5 is the data file to be read
//arg 6 is tvg "bram_wr_reg" name
//arg 7 is tvg "tvg_ctrl" name
//declarations

int main(int argc, char **argv){
    int i,j;
    int thr_cpu_ctrl,length=atoi(argv[2]),skip=atoi(argv[3]),stall=atoi(argv[4]);
    int bram_wr_reg,we,bram_addr,select_ram,data=0;
     
    FILE *thr_cpu_ctrl_file,*bram_wr_reg_file;
    FILE *data_file;//,*ioreg_mode;
//    char thr_cpu_ctrl_cmd[800],bram_wr_reg_cmd[800];
//    char ioreg_mode_cmd[700]; 
    char file[20];//argv[5];//"tvg.dat";
    memset(file,0,20);
    memcpy(file,argv[5],strlen(argv[5]));

    char tmp[10],tmp1[100],tmp2[100];

//write value to thr_cpu_ctrl setting length,skip and stall
    
    thr_cpu_ctrl=(length << 20) + (stall << 10) + (skip);

    sprintf(tmp,"%s",file);
    data_file=fopen(tmp,"r");
    sprintf(tmp1,"/proc/%s/hw/ioreg/%s",argv[1], argv[7]);
    thr_cpu_ctrl_file=fopen(tmp1,"wb");      
    sprintf(tmp2,"/proc/%s/hw/ioreg/%s",argv[1], argv[6]);
    bram_wr_reg_file=fopen(tmp2,"wb");

    if(data_file == NULL || thr_cpu_ctrl_file == NULL || bram_wr_reg_file == NULL)
    {
        printf("Error: could not open data file\n");
        return 1;
    }

    i=0;

    fwrite(&thr_cpu_ctrl,4,1,thr_cpu_ctrl_file);
   
    for(i=0;i<length;i++)
    {
        fscanf(data_file,"%s\n",tmp);
        bram_addr=i;
        data=atof(tmp)*32767;
//        data=atof(tmp);
        select_ram=3;
        we=1;
        
        for(j=0;j<16;j++)
            bram_wr_reg=((we << 29) + (select_ram << 27) + (i << 18) + (data));
           
        printf("%d %d %d\n",i, data, bram_wr_reg);
        fwrite(&bram_wr_reg,4,1,bram_wr_reg_file);
        fseek(bram_wr_reg_file,0,SEEK_SET); 
    }

    we=0;
    select_ram = 0;
    i = 0; data = 0;
    bram_wr_reg=((we << 29) + (select_ram << 27) + (i << 18) + (data));

    fwrite(&bram_wr_reg,4,1,bram_wr_reg_file);
    fseek(bram_wr_reg_file,0,SEEK_SET); 

    fclose(bram_wr_reg_file);
    fclose(thr_cpu_ctrl_file);      
    fclose(data_file); 
    return 0;

}
