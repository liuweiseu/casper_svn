//Last modified: 26. Sep 08
//Fills bram which controls beam / polarization switcher for BEE2 SETI spec
//The first argument specifies one of 5 possible observation modes
//The second argument is the number of times the sequence should be repeated
//In each case the amount of time is equally divided among beams/pols used
//Format:
// 7beam2pol n_cycles
// 7beam1pol n_cycles pol_sel
// 1beam2pol n_cycles beam_num
// 1beam1pol n_cycles beam_num pol_sel
// custom n_cyles beam_num(0) pol_sel(0) beam_num(1) pol_sel(1) ...
//
// n_cycles - number of repeatitions of sequence
// beam_num - beam number
// pol_sel - Flags polarization choice for the beam_num specified immediately before
// 0 = LHP, 1 = RHP, 2 = both
// For custom mode the desired beams and the choice of polarization are listed in any order after specifying the n_cyles

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BEAMDWL 0
#define IP_REM "192.168.2.6"

#define IN00 0xEE
#define IN01 0xED
#define IN02 0xEB
#define IN03 0xE7
#define IN04 0xDE
#define IN05 0xDD
#define IN06 0xDB
#define IN07 0xD7
#define IN08 0xBE
#define IN09 0xBD
#define IN10 0xBB
#define IN11 0xB7
#define IN12 0x7E
#define IN13 0x7D
#define IN14 0x7B
#define IN15 0x77

int main(int argc, char **argv){
    int i,j, k=0;
    int index=0;
    int bram_wr_reg;
    int n_beams, n_pol, n_cycles, dwelltime, max_add, len_msg; 
    int bram_addr, we;
    int bram_len=2048;
    int beams[7], pol[7], mapping[14];
    int bram[bram_len];

    int socknum, c, numbytes = 0;
    int packtype = 1;
    int jump, shift=0;
    char reqpack[1000], recvpack[1032];
    struct sockaddr_in sock_rem;

//Initialize beam and pol arrays

    for(j = 0; j < 7; j++)
    {
        beams[j] = 0;
        pol[j] = 255;
     }

/*Define mapping between sequence number and actual beam/pol
 *  mapping[0] -> mapping[6] must be same polarization
 *  mapping[7] -> mapping[13] must be other polarization
 *  Hardware configuration: 
 *  IN1, IN3... IN13 = polA
 *  IN2, IN4... IN14 = polB
 */
    mapping[0] = IN01;
    mapping[1] = IN02;
    mapping[2] = IN03;
    mapping[3] = IN04;
    mapping[4] = IN05;
    mapping[5] = IN06;
    mapping[6] = IN07;
    mapping[7] = IN08;
    mapping[8] = IN09; 
    mapping[9] = IN10;
    mapping[10] = IN11;
    mapping[11] = IN12;
    mapping[12] = IN13;
    mapping[13] = IN14;

/*PARSE MODE STRING AND SET PARAMETERS*/
    
   if(!strncmp(argv[1], "7beam2pol", 16))
   {
       printf("7 beam, 2 pol\n");
       n_beams = 7;
       n_pol = 2;
       
       n_cycles = atoi(argv[3]);

       printf("Number of beams: %d\n Number of pols: %d\n Number of cycles: %d\n", n_beams, n_pol, n_cycles);
       
       for(j = 0; j < 7; j++)
       {
           beams[j] = 1;
           pol[j] = 2;
       }
   }
   else if(!strncmp(argv[1], "7beam1pol", 16))
   {
       n_beams = 7;
       n_pol = 1;
       n_cycles = atoi(argv[3]);
       
       for(j = 0; j < 7; j++)
       {
           beams[j] = 1;
           pol[j] = atoi(argv[4]);
       }
   }
   else if(!strncmp(argv[1], "1beam2pol", 16))
   {
       
       n_beams = 1;
       n_pol = 2;
       n_cycles = atoi(argv[3]);
       
       beams[atoi(argv[4])] = 1;
       pol[atoi(argv[4])] = 2;
   }
   else if(!strncmp(argv[1],"1beam1pol", 16))
   {
       n_beams = 1;
       n_pol = 1;
       n_cycles = atoi(argv[3]);
       
       beams[atoi(argv[4])] = 1;
       pol[atoi(argv[4])] = atoi(argv[5]);
   }
   else if(!strncmp(argv[1], "custom", 16))
   {
       n_beams = (argc - 3)/2;
       n_cycles = atoi(argv[3]);

       for(i = 4; i < argc; i = i + 2)
       {
           beams[atoi(argv[i])] = 1;
           pol[atoi(argv[i])] = atoi(argv[i+1]);

           printf("Beam: %d Pol: %d \n", atoi(argv[i]), atoi(argv[i+1]));
           for(j = 0; j < 7; j++)
           {
               if(pol[j] == 0 || pol[j] == 1)
                  n_pol = n_pol + 1;
               else if(pol[j] == 2)
                  n_pol = n_pol + 2;
               else
                   printf("Invalid polarization flag.\n");
            }
        }
   }
   else if(!strncmp(argv[1], "help", 16))
   {
	printf("Usage: MODE DWELLFLAG DWELLTIME/NUM_CYCLES [BEAM] [POL]\n");
        printf("Possible MODE strings: 7beam2pol, 7beam1pol, 1beam2pol, 1beam1pol, custom\n");
	printf("DWELLFLAG determines how dwelltime determined. Probably want 0. See doc.\n");
	printf("Valid values for BEAM = 0-6.\n");
	printf("POL selector: 0 = L, 1 = R, 2 = both\n");
	printf("Example: > a.out 1beam2pol 0 10 5\n");
	printf("will switch between two polarizations of beam 5 every 10 sync pulses\n");
   }
   else
       printf("Unrecognized format specifier\n");

/*CALCULATE DWELLTIME AND MAX_ADD*/

/*dwelltime = number of sync pulses (i.e. number of BRAM addresses) per beam/pol
  max_add = max address to write in BRAM */

   if(atoi(argv[2]) == 1) //Parse "dwell mode"
	dwelltime = bram_len/(n_beams*n_pol*n_cycles);
   else
   {    dwelltime = atoi(argv[3]);
	n_cycles = 1;
   }

   max_add = dwelltime*n_beams*n_pol*n_cycles;
      

   printf("Dwelltime: %d\nMax address: %d\n", dwelltime, max_add);
   if(max_add == 0 || max_add > 2047)
	printf("Invalid dwell or num_cycles parameter.\n"); 

/*SELECT CYCLE BEAMS OR POLS */

   if(BEAMDWL==1)
   {    jump=1;
	shift=7;
   }
   else
   {	jump=2;
	shift=1;
   }

/*FILL BRAM ARRAY WITH CONTROL VALUES*/

   for(i = 0; i < n_cycles; i++)
   {
#ifdef VERBOSE
       printf("Cycle: %d\n", i);
#endif       
       for(j = 0; j < 7; j++)
       {
#ifdef VERBOSE
            printf("Beam %d\n", j);
#endif           
            if(beams[j] == 1)
            {    
#ifdef VERBOSE
                printf("Pol: %d\n", pol[j]);
#endif
               if(pol[j] == 0 || pol[j] == 2)
               {
                   for(k = 0; k < dwelltime; k++)
                        bram[k + index] = mapping[jump*j];
               
                   index = index + dwelltime;
               }
               
               if(pol[j] == 1 || pol[j] == 2)
               {     
                   for(k = 0; k < dwelltime; k++)
                        bram[k + index] = mapping[jump*j+shift];
               
                   index = index + dwelltime;
               }
            }
       }
    }
#ifdef NOIBOB
	for(i=0; i<max_add; i++)
		printf("%d %d %x\n", i, bram[i], bram[i]);
#endif

#ifndef NOIBOB
/* Sending commands to iBOB UDP server */

/* Open socket */

        socknum = socket(PF_INET, SOCK_DGRAM, 0);     //SOCK_DGRAM for UDP
                if(socknum < 0)
                        printf("A socket is not open.\n");

/* Define protocol, port, and IP address */
        sock_rem.sin_family = AF_INET; //Remote socket
        sock_rem.sin_port = htons(7);
        inet_pton(AF_INET, IP_REM, &(sock_rem.sin_addr)); //Set iBOB IP

/*Connect */
       c = connect(socknum, (struct sockaddr *)&sock_rem, sizeof(sock_rem) );
                if(c<0)
                        printf("Connect failed.\n");

/*Send values for switchyard BRAM to iBOB*/

   for(i=0; i <= max_add; i++)
   {   
       len_msg = sprintf(reqpack, "%c%c%c%c%c%c%c%c%s %d %d \n %s %d \n", '0' ,'0','0','0','0','0','0','0', "bramw beam_switcher/switchyard_BRAM", i, bram[i], "bramr beam_switcher/switchyard_BRAM", i);

#ifdef VERBOSE
        printf("Length of message: %d\n", len_msg);
#endif
        reqpack[0] = 255;

/* Send */
        numbytes = send(socknum, reqpack, len_msg*sizeof(char),0);

/* Receive loop*/
	packtype = 1; //Set back to 1 to enter loop

        while(packtype == 1) //If 1, more packets coming; if 2, last/only
        {
                numbytes = recv(socknum, recvpack, 1032*sizeof(char), 0);
                
		packtype = (int) recvpack[6]; //Read type from header

                for(j = 0 ; j < (numbytes - 8); j++)  //Rotate bytes to clear header
                        recvpack[j] = recvpack[j+8];
                recvpack[numbytes-8] = 0; //Append null char
                
                printf("%s", recvpack);
        }

#ifdef VERBOSE
        printf("Num bytes sent: %d %s\n", numbytes, recvpack);
#endif

     }
 
/*Send value for 'max_addr' register to iBOB*/

       len_msg = sprintf(reqpack, "%c%c%c%c%c%c%c%c%s %d \n", '0' ,'0','0','0','0','0','0','0', "regw beam_switcher/max_addr", max_add);

        reqpack[0] = 255;

/* Send */
        numbytes = send(socknum, reqpack, len_msg*sizeof(char),0);
#ifdef VERBOSE
                printf("Num bytes sent: %d\n", numbytes);
#endif

/*Close socket*/
        close(socknum);
#endif 
   return 0;
}
