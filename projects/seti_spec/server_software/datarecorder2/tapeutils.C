//#include "dr2.h"
#include "tapeutils.h"

int main(int argc, char ** argv) {

    int fd;
    int * cs;
    int i, retval;
    long numcomplex;
    int channel;

    if ((fd = open(argv[1],O_RDONLY,0)) == -1) {
        fprintf(stderr,"Error opening tape device\n");
        exit(1);
    }

    channel    = atoi(argv[2]);
    numcomplex = atoi(argv[3]);

    retval = GetTapeData(fd, channel, numcomplex, cs);

    if(retval) {
        fprintf(stderr, "Could not allocate or fill array\n");
        exit(1);
    } 

    for (i = 0; i < numcomplex; i++) {
        fprintf(stderr,"%ld %d %d\n",i,cs[i*2], cs[(i*2)+1]);
    }
}
