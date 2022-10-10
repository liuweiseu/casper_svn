#ifndef SETISPEC_H
#define SETISPEC_H

#define RECVPORT 33001
#define PAYLOADSIZE 4096 
#define DATAFILESIZE 1024000
#define COARSEBINS 4096
#define NUMFILES 1
#define BINS 134217728.0
#define BANDBEGIN 100.0
#define BANDEND 300.0
#define U32 unsigned int
#define U8 unsigned short

int delta_pfb;
int delta_fft;
float tone;
int port;

void kill_ct(void);
void kill_pfb(void);
void kill_fft(void);
void kill_thr(void);
void kill_bee(void);

void ddc_conf(void);
void ct_conf(void);
void pfb_conf(void);
void fft_conf(void);
void thr_conf(void);

void restart_thr(void);
void restart_fft(void);
void restart_ct(void);
void restart_pfb(void);
void restart_bee(void);

void write_pfb_fft(void);
int read_pfb_fft_compare(void);
int recv_data_write(void);
int compare_tones(int);

int freq_bin(float,float,float,float);

void compare_pfb_tvg(void);
void fill_tvg(void);
void set_tvg(float);
void tvg_test(int,float,int);

int setamp(float);
int setfreq(float);

int rnd(float x);

int *GetSpectra(void);

struct data_vals{
    unsigned int raw_data;
    unsigned int overflow_cntr;
};

#endif
