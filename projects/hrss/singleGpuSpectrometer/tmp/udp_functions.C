#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sched.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "edtinc.h"
#include "udp_functions.h"

static unsigned int slice(unsigned int value,unsigned int width,unsigned int offset);

static pthread_t fill_buf_thread, packet_buf_thread, get_beam_thread, get_bee_config_thread;

packet_buffer packet_buffer_ring[RINGBUFFSIZE];
spectra_buffer spectra_buffer_ring[RINGBUFFSIZE];

udp_data seti_udp_data;

int run_print_buf=1;
int run_fill_buf=1;
int run_packet_buf=1;
int run_get_beam=1;
int run_get_bee_config=1;

unsigned long total_spectra=0;

unsigned long int beam_number;
pthread_mutex_t beam_number_mutex;

char bee_config_buffer[BEE_CONFIG_PACKET_SIZE];
pthread_mutex_t bee_config_buffer_mutex;

void initialize_buffers()
{
    int i;
    pthread_mutex_init(&beam_number_mutex, NULL);
    
    for(i=0;i<RINGBUFFSIZE;i++)
    {
        // initialize packet buffer ring
        packet_buffer_ring[i].data = calloc(PAYLOADSIZE,sizeof(int));
        packet_buffer_ring[i].full = 0;
        pthread_mutex_init(&packet_buffer_ring[i].buffer_mutex, NULL);
        
        // initialize spectra buffer ring
        spectra_buffer_ring[i].data = calloc(NUM,sizeof(int));
        spectra_buffer_ring[i].full = 0;
        pthread_cond_init(&spectra_buffer_ring[i].buffer_filled_cond, NULL);
        pthread_mutex_init(&spectra_buffer_ring[i].buffer_mutex, NULL);
        pthread_mutex_init(&spectra_buffer_ring[i].sizeofbuf_mutex, NULL);
        pthread_mutex_init(&spectra_buffer_ring[i].full_mutex, NULL);
        
    }
}

void close_buffers()
{
    int i;
    for(i=0;i<RINGBUFFSIZE;i++)
    {
        free(packet_buffer_ring[i].data);
        free(spectra_buffer_ring[i].data);
    }    
}

void *fill_buf( void *num )
{
    int i=0,j=0;
    long int record_cntr = 0;
    long int beam_number_tmp = 0;
    long int beam_number_prev = 0;
    int k=0;
    
    int pfb_bin_prev = 0;
    int pfb_bin = 0;
    int fft_bin = 0;
    int over_thresh = 0;
    int blank = 0;
    int event = 0;
    int pfb_fft_power = 0;
    
   
 
    struct data_vals *data_ptr;
    struct data_vals *buffer_ptr;
    
    fprintf(stderr, "fill_buf thread launched\n");
    
    pthread_mutex_lock( &(spectra_buffer_ring[0].buffer_mutex) );
    buffer_ptr = (struct data_vals *)spectra_buffer_ring[0].data;
    pthread_mutex_unlock( &(spectra_buffer_ring[0].buffer_mutex) );
    
    while( run_fill_buf )
    {
        while( packet_buffer_ring[k].full == 1 )
        {
            pthread_mutex_lock( &(spectra_buffer_ring[i].buffer_mutex) );
            //printf("Accessing buffer %d\n",i);
            
            pthread_mutex_lock( &(packet_buffer_ring[k].buffer_mutex) );
            
            data_ptr = (struct data_vals *)packet_buffer_ring[k].data;
            packet_buffer_ring[k].full = 0;
            
            for(j=0;j<PAYLOADSIZE/8;j++)
            {
                unsigned int fields = ntohl(data_ptr->raw_data);
                fft_bin = slice(fields,15,0);
                pfb_bin = slice(fields,12,15);
                over_thresh = slice(fields,1,27);
                blank = slice(fields,3,28);
                event = slice(fields,1,31);
                pfb_fft_power = ntohl(data_ptr->overflow_cntr); 
 
//		pfb_bin = (pfb_bin + 2048) % 4096;
              /* 
                if(pfb_bin > 2047)
                {
                    pfb_bin = pfb_bin-2048;
                }
                else
                {
                    pfb_bin = pfb_bin+2048;
                }
              */  
                //printf(":: record %d buffer %d pfbBinPrev %d pfbBin %d\n",record_cntr,i,pfb_bin_prev,pfb_bin);
                
                if( pfb_bin < pfb_bin_prev )
                {
                    printf("got spectra %d\n",i);
                    total_spectra++;
                    
                    pthread_mutex_lock( &spectra_buffer_ring[i].full_mutex );
                    
                    spectra_buffer_ring[i].full = 1;
                    pthread_cond_signal(&spectra_buffer_ring[i].buffer_filled_cond);
                    
                    pthread_mutex_unlock( &spectra_buffer_ring[i].full_mutex );
                    
                    pthread_mutex_unlock( &(spectra_buffer_ring[i].buffer_mutex) );
                    
                    //adding in header data (go back to start of this buffer)
                    buffer_ptr = (struct data_vals *)spectra_buffer_ring[i].data;

                    //copy in number of records
                    memcpy( buffer_ptr, &(record_cntr), 8 );
                    buffer_ptr++;

                    //copy in beam number
                    //memcpy( buffer_ptr, &(beam_number_tmp), 8 );
                    memcpy( buffer_ptr, &(beam_number_prev), 8 ); //delay
		    beam_number_prev = beam_number_tmp;  //adding in delay for beam
                    buffer_ptr++;

                    //copy in bee config data header
                    memcpy( buffer_ptr, bee_config_buffer, BEE_CONFIG_PACKET_SIZE); 
                    
                    //set buffer size
                    pthread_mutex_lock( &(spectra_buffer_ring[i].sizeofbuf_mutex) );
                    spectra_buffer_ring[i].buffer_size=(unsigned int)record_cntr;
                    pthread_mutex_unlock( &(spectra_buffer_ring[i].sizeofbuf_mutex) );
		    
                    i = (i + 1) % 4;
                    
                    pthread_mutex_lock( &(spectra_buffer_ring[i].buffer_mutex) );
                    buffer_ptr = (struct data_vals *)spectra_buffer_ring[i].data;

                    //add space for buffer size and beam number (to be added later)
                    buffer_ptr+=HEADER_SIZE_IN_RECORDS; 
                    record_cntr = 0;

		    //get beam number
                    //pthread_mutex_lock(&beam_number_mutex);
		    //beam_number_tmp = beam_number;
                    //pthread_mutex_unlock(&beam_number_mutex);

                }
                
		//get beam number if pfb_bin equals 1024 
		if(pfb_bin == 1024)
		{
		    pthread_mutex_lock(&beam_number_mutex);
		    beam_number_tmp = beam_number;
		    pthread_mutex_unlock(&beam_number_mutex);
		}

                memcpy( buffer_ptr, data_ptr, 8 );
                
                buffer_ptr++;		
                data_ptr++;	
                record_cntr++;
                
                pfb_bin_prev = pfb_bin;
                
                pthread_mutex_lock( &(spectra_buffer_ring[i].sizeofbuf_mutex) );
                spectra_buffer_ring[i].buffer_size=(unsigned int)record_cntr;
                pthread_mutex_unlock( &(spectra_buffer_ring[i].sizeofbuf_mutex) );
            }
            
            pthread_mutex_unlock( &(packet_buffer_ring[k].buffer_mutex) );
            k = (k + 1) % RINGBUFFSIZE;
            pthread_mutex_unlock( &(spectra_buffer_ring[i].buffer_mutex) );
        }
        
        //   pthread_mutex_unlock( &(packet_full_mutex[i]) );
        
    }
    
    fprintf(stderr, "fill_buf thread received Ctrl-C\n");
    return NULL;
}

void get_next_buf(edt_buffer *buf, int i)
{
    pthread_mutex_lock( &spectra_buffer_ring[i].full_mutex );
    while( spectra_buffer_ring[i].full == 0) 
    {
        pthread_cond_wait(&spectra_buffer_ring[i].buffer_filled_cond, &spectra_buffer_ring[i].full_mutex);
    }
    //printf("got this far\n");
    pthread_mutex_lock( &spectra_buffer_ring[i].sizeofbuf_mutex );
    buf->data = spectra_buffer_ring[i].data;
    buf->buffer_size = spectra_buffer_ring[i].buffer_size*RECORD_SIZE_IN_BYTES+HEADER_SIZE_IN_BYTES;
    //fprintf(stderr, "returning next buffer size %d records %d record size %d header size %d\n", buf->buffer_size);
    spectra_buffer_ring[i].full = 0;
    spectra_buffer_ring[i].buffer_size = 0;
    pthread_mutex_unlock( &spectra_buffer_ring[i].sizeofbuf_mutex );
    pthread_mutex_unlock( &spectra_buffer_ring[i].full_mutex );
    return;
}

void *print_buf( void *num)
{
    int i = 0;
    FILE *fp;
    char filename_str[200];
    edt_buffer nextBuffer;
    size_t byteswritten;
    
    while(run_print_buf)
    {
        get_next_buf(&nextBuffer, i);
        sprintf((char *)filename_str,(const char *)"datafiles/spectra/data%d.dat",i);
        fp=fopen((const char *)filename_str,"wb");
        //printf("Writing file %s: size %u pointer %d\n",filename_str, nextBuffer.buffer_size, (int)nextBuffer.data); 
        byteswritten = fwrite(nextBuffer.data,nextBuffer.buffer_size,1,(FILE *)fp);
        //perror("Error writing\n"); 
        
        fclose(fp);
        
        
        i = (i + 1) % RINGBUFFSIZE;
        
    }
    fprintf(stderr, "print_buf thread received Ctrl-C\n");
    return NULL;
}

int udp_open()
{    
    //initialize udp
    
    if ((seti_udp_data.sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    
    seti_udp_data.recv_addr.sin_family = AF_INET;              // host byte order
    seti_udp_data.recv_addr.sin_port = htons(RECVPORT);        // short, network byte order
    seti_udp_data.recv_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with recv IP
    memset(seti_udp_data.recv_addr.sin_zero, '\0', sizeof seti_udp_data.recv_addr.sin_zero);
    
    if (bind(seti_udp_data.sockfd, (struct sockaddr *)&(seti_udp_data.recv_addr), sizeof (seti_udp_data.recv_addr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    
    seti_udp_data.addr_len = sizeof (seti_udp_data.send_addr);
    return 1;
}



int udp_close()
{
    close(seti_udp_data.sockfd);
    return 0;
} 


static unsigned int slice(unsigned int value,unsigned int width,unsigned int offset){
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}

static void *udp_get_packet(void *buf)
{
    ssize_t numbytes; 
    
    //printf("port %d\n",seti_udp_data.recv_addr.sin_port);
    if ((numbytes = recvfrom(seti_udp_data.sockfd, buf, PAYLOADSIZE , 1,
                             (struct sockaddr *)&(seti_udp_data.send_addr), &(seti_udp_data.addr_len))) == -1)
    {
        perror("recvfrom");
        exit(1);
    }
    
    return buf; 
}


void *packet_buf(void *num)
{
    int i = 0;
    
    fprintf(stderr, "packet_buf thread launched\n");
    
    while(run_packet_buf)
    {
        pthread_mutex_lock( &packet_buffer_ring[i].buffer_mutex );
        udp_get_packet(packet_buffer_ring[i].data);       
        packet_buffer_ring[i].full = 1;
        pthread_mutex_unlock( &packet_buffer_ring[i].buffer_mutex );
        i = (i+1) % 4;
    }
    fprintf(stderr, "packet_buf thread received Ctrl-C\n");
    return NULL;
}

void *get_beam( void *num )
{
    //int beam_number = 0;
    char *hexval_str;
    char reqpack[1000];
    char recvpack[1032];
    int s, c, sb;
    //int s, c, b, sb, l, n;
    int len_msg,i;
    int packtype = 1; //Must be one to enter receive loop
    struct sockaddr_in sock_rem;
    
    /*Prepare request packet*/
    
    for(i = 0; i < 1000; i++)
    {
        reqpack[i]=0;
        recvpack[i]=0;
    }
    
    len_msg = sprintf(reqpack, "%c%c%c%c%c%c%c%c%s \n",'0','0','0','0','0','0','0','0',"regread beam_switcher/beam_number");
    
    reqpack[0] = (char )255; //Set first byte of request packet header
    
    /* BEGIN UDP communication */
    /* Define protocol, port, and IP address */
    sock_rem.sin_family = AF_INET; //Remote socket
    sock_rem.sin_port = htons(7);
    inet_pton(AF_INET, "192.168.2.6", &(sock_rem.sin_addr)); //Set iBOB IP
    
    /* Open socket */
    s = socket(PF_INET, SOCK_DGRAM, 0);     //SOCK_DGRAM for UDP
    if(s < 0) printf("A socket is not open.\n");
    /*Connect */
    c = connect((int)s, (struct sockaddr *)&sock_rem, sizeof(sock_rem) );
    if(c<0) printf("Connect failed.\n");

    
    
    /* Receive loop*/
    while(run_get_beam == 1)
    {
        /* Send */
        sb = (int)send(s, reqpack, len_msg*sizeof(char),0);
        while(packtype == 1) //If 1, more packets coming; if 2, last/only
        {
            sb = (int)recv(s, recvpack, 1032*sizeof(char), 0);
            packtype = (int) recvpack[6]; //Read type from header
            
            for(i = 0 ; i < (sb - 8); i++)  //Rotate bytes to clear header
                recvpack[i] = recvpack[i+8];
            recvpack[sb-8] = 0; //Append null char
            
            hexval_str = strtok(recvpack," ");
            hexval_str = strtok(hexval_str,"x");
            hexval_str = strtok(NULL,"x");
            
            pthread_mutex_lock(&beam_number_mutex);
            if(hexval_str!=NULL)
            {
                sscanf(hexval_str,"%lx",&beam_number);
            }
            pthread_mutex_unlock(&beam_number_mutex);
            usleep(100000);
        }
        packtype = 1;
    }
    
    fprintf(stderr, "get_beam thread received Ctrl-C\n");
    
    /*Close socket*/
    close(s);
    return NULL;
}

void *get_bee_config( void *num )
{
    int i;
    int numbytes;

    pthread_mutex_lock(&bee_config_buffer_mutex);
    for(i=0;i<512;i++)
    {
        bee_config_buffer[i] = '\0';
    }
    pthread_mutex_unlock(&bee_config_buffer_mutex);

    int sockfd;
    struct sockaddr_in recv_addr;
    struct sockaddr_in send_addr;
    socklen_t addr_len;

    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(BEE_CONFIG_RECV_PORT);
    recv_addr.sin_addr.s_addr = INADDR_ANY;
    memset(recv_addr.sin_zero, '\0', sizeof recv_addr.sin_zero);

    if(bind(sockfd, (struct sockaddr *)&recv_addr, sizeof recv_addr) == -1)
    {
        perror("bind");
        exit(1);
    }

    addr_len = sizeof send_addr;

    while(run_get_bee_config)
    {
        pthread_mutex_lock(&bee_config_buffer_mutex);
        if((numbytes = (int)recvfrom(sockfd,bee_config_buffer,BEE_CONFIG_PACKET_SIZE, 1,
                (struct sockaddr *)&send_addr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }

        printf("%s \n",bee_config_buffer);
        pthread_mutex_unlock(&bee_config_buffer_mutex);
    }
    
    fprintf(stderr, "get_bee_config thread received Ctrl-C\n");
    
    close(sockfd);

    return NULL;
}

void launch_buf_threads()
{
    int *val = 0;
    pthread_create( &get_beam_thread, NULL, get_beam, (void *)val);
    pthread_create( &get_bee_config_thread, NULL, get_bee_config, (void *)val);
    pthread_create( &packet_buf_thread, NULL, packet_buf, (void *)val);
    pthread_create( &fill_buf_thread, NULL, fill_buf, (void *)val);
}

void join_buf_threads()
{
    run_fill_buf=0;
    pthread_join( fill_buf_thread, NULL );
    run_packet_buf=0;
    pthread_join( packet_buf_thread, NULL );
    run_get_beam=0;
    pthread_join( get_beam_thread, NULL );
    run_get_bee_config=0;
    pthread_join( get_bee_config_thread, NULL );
}



