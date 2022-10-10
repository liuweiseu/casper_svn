#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>

typedef union {
    unsigned long data[4];
    unsigned char beam_byte[16];
} destriped_t;

long word_cnt = 0;
void sigterm_func(int);

main() {
    int stripe_cnt = 0;
    int i,j;
    int beam_file[16];
    unsigned char fake_beam_file[16];
    unsigned long data_in;
    unsigned long data_masked;
    unsigned long data_mask[4] = {0x03030303,	// beams 0,4,8,12
                                  0x0c0c0c0c,	// beams 1,5,9,13
                                  0x30303030,	// beams 2,6,10,14
                                  0xc0c0c0c0};	// beams 3,7,11,15
    destriped_t destriped;

    char * beam_file_name[16] = {"beam0", "beam2", "beam3", "beam3",
                                 "beam4", "beam5", "beam6", "beam7",
                                 "beam8", "beam9", "beam10", "beam11",
                                 "beam12", "beam13", "beam14", "beam15"};
    /*
    	for(i=0; i < 16; i++) {
    		beam_file[i] = (beam_file_name[i], O_RDWR);
    	}
    */
    signal(SIGINT, sigterm_func);

    // For each 32 bit data_in word we do:
    //	- 4 bitwise ANDs (culling out 4 bytes (beams) each)
    // 	- 4 bitwise ORs (each one moving 4 bytes (beams) to
    // 	  a beam specific word
    //	- no shifting
    // For every 4 data_in words we write each of 16 beam specific
    // words to their corresponding memory mapped disk buffer.
    while(1) {
        word_cnt++;
        stripe_cnt++;
        for(i=0; i<4; i++) {
            data_masked = data_in & data_mask[i];
            destriped.data[i] = (data_masked >> 2) | destriped.data[i];
        }
        if(stripe_cnt = 3) {
            for(j=0; j<16; j++) {
                // write out destriped.beam_byte[j]
                fake_beam_file[j] = destriped.beam_byte[j];
            }
            stripe_cnt = 0;
        }
        /*
        		if(clock()/1000000 > 60) {
        			printf("words processed per s = %ld, per 200 ns = %lf\n", word_cnt, 
        				((double)word_cnt/300000000));
        			exit(0);
        		}
        */

    }
}

void sigterm_func(int) {
    printf("words processed per s = %ld, per 200 ns = %lf\n", word_cnt,
           ((double)word_cnt/300000000));
    exit(0);
}
