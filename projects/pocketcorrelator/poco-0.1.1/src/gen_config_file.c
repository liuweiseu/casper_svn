#include <stdio.h>     /* Standard input/output definitions */
#include <string.h>    /* String function definitions */
#include <unistd.h>    /* UNIX standard function definitions */
#include <fcntl.h>     /* File control definitions */
#include <errno.h>     /* Error number definitions */
#include <termios.h>   /* POSIX terminal control definitions */
#include <signal.h>    /* Signal functions*/
#include <sys/time.h>  /* Time... is on my side... */
#include <time.h>      /* Yes it is */
#include <math.h>

int setbits_str(int , int , int , char*);
int setbits(int , int , int , int);
void bin_print(int, int);
void exit(int status);

int main(int argc, char **argv)
{
  int eqvalue = 0;
  int k=0, l=0;
  int coeff=100;
  FILE *fp;
  char filename[100];
  int c;

  char *fft_shift = "0x03ff";
  char *ip_address = "169 254 128 10";
  int sync_gen_period = 2662400;
  int eq_level = 52000;

  for (c=1; c<argc; c++){
    if(strcmp(argv[c], "-h") == 0)
    {
      printf("Usage: %s [options]\n",argv[0]);
      printf("\t -a [ip address] : ip address to send packets to, use format \"169 254 128 10\", default: 169 254 128 10\n"); 
      printf("\t -f [fft shift] : fft stage shifting value, default: 0x03ff\n");
      printf("\t -s [sycn_gen/period] : sync gen period, default: 2662400\n");
      printf("\t -e [equalization] : set the equalization level, default: 52000\n");
      exit(0);
    } else if(strcmp(argv[c], "-f") == 0){
      fft_shift = argv[c+1];
      c++;
    } else if(strcmp(argv[c], "-s") == 0){
      sync_gen_period = atoi(argv[c+1]);
      c++;
    } else if(strcmp(argv[c], "-e") == 0){
      eq_level = atoi(argv[c+1]);
      c++;
    }
  }

//for(l=100;l<5000;l=l+100){
//for(l=480000;l<480001;l=l+100){
  sprintf(filename, "config%d.txt", eq_level);
  
  printf("%s\n", filename);
  if ((fp = fopen(filename, "w")) == NULL) {
    printf("\nerror couldn't open file\n");
  }
  
  fprintf(fp, "regwrite ctrl_sw %s\n", fft_shift);
  fprintf(fp, "regread ctrl_sw\n");
  fprintf(fp, "regwrite sync_gen/period %i\n", sync_gen_period);
  fprintf(fp, "regread sync_gen/period\n");
  
  for(k=0;k < 1024;k++){
    eqvalue = setbits(eqvalue, 17, 1, 1);
    eqvalue = setbits(eqvalue, 0, 10, eq_level);
    eqvalue = setbits(eqvalue, 20, 10, k);
    bin_print(eqvalue, 32);
    
    fprintf(fp, "regwrite eq_coeff %d\n", eqvalue);
    fprintf(fp, "regread eq_coeff\n");
  }
  
  fprintf(fp, "startudp %s 6969\n",ip_address);
  
  fclose(fp);

//}
//	  eqvalue = setbits(eqvalue, 0, 10, 1023);
//	  bin_print(eqvalue, 32);
//  printf("%d --> %d\n", 53, setbits(53, 3, 4, 9));
//  printf("%d --> %d\n", 53, setbits_str(53, 3, 4, "1001"));

  return 0;
}


// setbits_str: same as setbits, except you specify the arbitrary bit pattern to set as a string of 1's and 0's
int setbits_str(int initialvalue, int startbit, int vallength, char* val)
{
  // convert val to integer
  int ival = 0;
  int i, tmp;
  for (i = 0; i < vallength; i++)
    ival += (((int)(val[i]))-48)*(1<<(vallength-1-i));
  return setbits(initialvalue, startbit, vallength, ival);
}

// setbits: sets an arbitrary bit pattern of arbitrary length at an arbitrary position in a 32 bit number
//
// assumes register is 32 bits wide
// initialregvalue: current value of 32-bit register
// startbit: starts from least significant bit (LSB is 0)
// vallength: number of bits you want to change
// val: the integer representation of the bits you want to set
int setbits(int initialvalue, int startbit, int vallength, int val)
{
  int newregvalue = 0;
  int i;
  for (i = 31; i >= 0; i--)
  {
    if ((i > startbit+(vallength-1)) || (i < startbit)) // we're on a bit that isn't being modified, so just copy it
      newregvalue += (1<<i)*( (initialvalue >> i) & 1 );
    else // we're on a bit that is being modified
      newregvalue += (val << (startbit)) & (1<<i);
  }
  return newregvalue;
}

void bin_print(int x, int n_bits)
// function to print decimal numbers in binary format 
{
int j;
//printf("no. 0x%08x in binary \n",x);
//for(j=n_bits-1; j>=0;j--){
//printf("bit: %i = %i\n",j, (x>>j) & 01);
//}
for(j=n_bits-1; j>=0;j--){
//printf("%i",(x>>j) & 01);
}

//printf("\n");

}
