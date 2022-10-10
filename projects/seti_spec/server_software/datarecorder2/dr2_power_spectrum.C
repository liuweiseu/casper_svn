#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <fcntl.h>
#include <strings.h>
#include <vector.h>

#include "tapeutils.h"

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

//const long num_r_in_d   = 8;
//const long r_BufSize    = 1024*1024;
//const long HeaderSize   = 4*1024;
//const long d_BufSize    = (r_BufSize + HeaderSize) * num_r_in_d;

void four1(float data[], unsigned long nn, int isign);

int main(int argc, char *argv[]) {

    int tapefd;
    //unsigned short *frame;
    unsigned short bit;
    long count;
    long atbyte;
    long i, j, k;
    long readbytes;
    unsigned short testbyte;
    unsigned long numcomplex;
    float *data = NULL;
    std::vector<float> data_v;
    std::vector<float>::iterator iter;
    long dataindex;
    float power;
    int channel_i;
    int smooth_factor = 1;
    int sum_spectra = 16384;
    //int sum_spectra = 1024;
    int sum_spectra_counter;
    float summed_spectra[2048] = {0};

    int retval;

    bool OutputTimeDomain = false;

    char * base_file_name;
    char this_file_name[256];
    char buffer[4096];
    FILE * ps_fp = NULL;
    FILE * td_fp = NULL;

    // Get args

    if (argc != 4) {
        fprintf(stderr, "Usage: dr2_power_spectrum <data_filename> <numcomplex>\n");
        exit(1);
    }

    base_file_name = argv[2];
    sscanf(argv[3],"%ld",&numcomplex);

    //data = (float *) malloc (numcomplex * 2 * sizeof(float));
    // get and initialize array for complex samples
    data = (float *) malloc (numcomplex * 2 * sizeof(int));
    if(!data) return(1);
    for (i = 0; i < (numcomplex * 2); i++) data[i] = -1;


    // Open tape

    if ((tapefd = open(argv[1],O_RDONLY,0)) == -1) {
        fprintf(stderr,"Error opening tape device\n");
        exit(1);
    }


    //frame = (unsigned short *) malloc (d_BufSize);

    // This is set up for disk buffers, ie only 8 channels
    for (channel_i=0; channel_i < 8; channel_i++) {
        lseek(tapefd, 0L,  SEEK_SET);
        
        fprintf(stderr, "Extracting channel %d\n", channel_i);

    if (OutputTimeDomain) {
        sprintf(this_file_name, "time_domain_%s_%d", base_file_name, channel_i);
        td_fp = fopen(this_file_name, "w");
        if (!td_fp) {
            fprintf(stderr, "cannot open %s\n", this_file_name);
            exit(1);
        }
    }

    sprintf(this_file_name, "%s_%d", base_file_name, channel_i);
    ps_fp = fopen(this_file_name, "w");
    if (!ps_fp) {
        fprintf(stderr, "cannot open %s\n", this_file_name);
        exit(1);
    }

    for (sum_spectra_counter = 0; sum_spectra_counter < sum_spectra; sum_spectra_counter++) {

        static int m;
        //fprintf(stderr, "%d Extracting channel %d\n", m++, channel_i);
        //if (data) free (data);
        //retval = GetTapeData(tapefd, channel_i, numcomplex, data);
        data_v.resize(0);
        retval = GetDr2Data(tapefd, channel_i, numcomplex, data_v);
        if (retval) {
            fprintf(stderr, "GetTapeData %d \n", retval);
            perror(NULL);
            break;
            fprintf(stderr, "GetTapeData failed\n");
            exit(1);
        }

        if (OutputTimeDomain) {
            for (i = 0; i < numcomplex; i++) {
                fprintf(td_fp,"%ld %f %f\n",i,data[i*2], data[(i*2)+1]);
            }
        }

        //for (i = 0; i < (numcomplex * 2); i++) data[i] = data_v[i];
        //for (iter = data_v.begin(), i = 0; iter != data_v.end(); iter++, i++) data[i] = *iter;
        for (iter = data_v.begin(), i = 0; iter != data_v.end(); iter++, i++) data[i] = data_v[i];
        four1(data-1,numcomplex,1);

        k = 0;
        //for (i = 1; i < numcomplex; ) {
        for (i = numcomplex/2; i < numcomplex; i++) {
            //power = 0.0;
            //for (j = 0; j < smooth_factor; j++, i++) {
            //    power += (data[i*2] * data[i*2]) + (data[(i*2)+1] * data[(i*2)+1]);
            //    summed_spectra[k++] += power;
            //}
            summed_spectra[k++] += (data[i*2] * data[i*2]) + (data[(i*2)+1] * data[(i*2)+1]);
            //fprintf(ps_fp,"%ld %ld %f\n",k++,i,power);
        }
        // skip the DC bin (i = 1)
        for (i = 1; i < numcomplex/2; i++) {
            //power = 0.0;
            //for (j = 0; j < smooth_factor; j++, i++) {
            //    power += (data[i*2] * data[i*2]) + (data[(i*2)+1] * data[(i*2)+1]);
            //    summed_spectra[k++] += power;
            //}
            summed_spectra[k++] += (data[i*2] * data[i*2]) + (data[(i*2)+1] * data[(i*2)+1]);
            //fprintf(ps_fp,"%ld %ld %f\n",k++,i,power);
        }



    } // end for

    for (i = 0; i < numcomplex; i++) {
        fprintf(ps_fp,"%ld %ld %f\n",k++,i, summed_spectra[i]);
    }

        //if (td_fp) {
        //    fclose(td_fp);
        //    td_fp = NULL;
       // }
        if (ps_fp) {
            fclose(ps_fp);
            ps_fp = NULL;
        }
    } 

    if (data) free (data);
    close(tapefd);
    exit(0);
} // end main


void four1(float data[], unsigned long nn, int isign) {
    unsigned long n,mmax,m,j,istep,i;
    double wtemp,wr,wpr,wpi,wi,theta;
    float tempr,tempi;

    n=nn << 1;
    j=1;
    for (i=1;i<n;i+=2) {
        if (j > i) {
            SWAP(data[j],data[i]);
            SWAP(data[j+1],data[i+1]);
        }
        m=n >> 1;
        while (m >= 2 && j > m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }
    mmax=2;
    while (n > mmax) {
        istep=mmax << 1;
        theta=isign*(6.28318530717959/mmax);
        wtemp = sin(0.5*theta);
        wpr = -2.0*wtemp*wtemp;
        wpi = sin(theta);
        wr=1.0;
        wi=0.0;
        for (m=1;m<mmax;m+=2) {
            for (i=m;i<=n;i+=istep) {
                j=i+mmax;
                tempr=(float)(wr*data[j]-wi*data[j+1]);
                tempi=(float)(wr*data[j+1]+wi*data[j]);
                data[j]=data[i]-tempr;
                data[j+1]=data[i+1]-tempi;
                data[i] += tempr;
                data[i+1] += tempi;
            }
            wr=(wtemp=wr)*wpr-wi*wpi+wr;
            wi=wi*wpr+wtemp*wpi+wi;
        }
        mmax=istep;
    }
}

#undef SWAP
