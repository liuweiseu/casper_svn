#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char ** argv) {

    short word;
    short bit;
    int i,bitpos,numsamples,bits=0,words=0;
    FILE * fp;

    bitpos = atoi(argv[2]);
    numsamples = atoi(argv[3]);

    if ((fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Cannot open %s\n", argv[1]);
    }

    //for (i=0; i < 16; i++) {
    while (fread(&word, 2, 1, fp)) {
        bits++;
        words++;
        bit = ((((word << 15 - bitpos) >> 1) & 0x4fff)  >> 14);
        //bit = word << 15 - bitpos;
        //bit = bit >> 1;
        //bit = bit & 0x4fff;
        //bit = bit >> 14;
        fprintf(stderr, "%c ", bit == 1 ? '-' : '_');
        if (bits >= 32) {
            fprintf(stderr, "\n\n");
            bits = 0;
        }
        if (words >= numsamples) {
            fprintf(stderr, "\n");
            break;
        }
    }
}
