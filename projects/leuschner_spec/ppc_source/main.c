/* ********************** */
/* *                    * */
/* *    iBOB Start-up   * */
/* *                    * */
/* ********************** */

/* 2004 Pierre-Yves droz */
/* 2007 David MacMahon */

/* Main file */

#include "xparameters.h"
#include "xuartlite_l.h"
#include "xbasic_types.h"
#include "tinysh.h"
#include "lwip/udp.h"
#include "core_info.h"

#ifndef LINUX_ENABLE
#ifdef LWIP_ENABLE
// Xilinx timer stuff
#include "xtime_l.h"
// netif structure definition
#include "netif/xemacliteif.h"
#include "lwiputil.h"
#include "fifo.h"
// Defined in lwipinit.c
extern struct netif default_netif;
extern fifo_p input_fifo;
extern void lwip_tmr();

/* printmode switch, default = serial, 1 for tcp, 2 for tcp-sans-CR */
Xuint8 printmode=0;
Xuint8 immediatemode=0; // Tells bufferbyte whether to call send_buf immediately
#else // Not LWIP_ENABLE
/* #define away the lwip functions for NON-lwip builds */
#define xemacliteif_input(x)
#define lwip_tmr()
#define printmode 0
#endif // LWIP_ENABLE
#else // LINUX_ENABLE
#include "xcache_l.h"
#include <stdio.h>
#include <stdlib.h>
#define MAX_PACKET_SIZE 1200

int linux = 0;

/*****************************************************************
 * Linux-specfic data macros
 *****************************************************************/

#define SELECTMAP_FIFO_NUM_WORDS 128

static inline Xuint32 GET_STATUS()
{
  return XIo_In32(XPAR_OPB_SELECTMAP_FIFO_0_BASEADDR);
}
static inline Xuint8  SELECTMAP_STATUS(Xuint32 word)
{
  return (Xuint8)((word >> 24) & 0xff);
}

#define SELECTMAP_RFIFO_CNT(word) (Xuint8)((word >> 16) & 0xff)
#define SELECTMAP_WFIFO_CNT(word) (Xuint8)((word >> 8)  & 0xff)

/* Linux-specific prototypes */
int  inbyte();
void intr_selectmap(void);
void send_writeack(unsigned location, int val);
void send_selectmap_byte(unsigned char c);
unsigned char receive_selectmap_byte();
unsigned int receive_selectmap_int3();
void send_selectmap_int3(unsigned int data);
unsigned int receive_selectmap_int4();
void send_selectmap_int4(unsigned int data);

/* Globals */
/* ********************************* */
unsigned char last_byte_written;
#endif // LINUX_ENABLE

/* Prototypes */
/* ********************************* */
void outbyte(unsigned char c);
static void version_cmd(int , char**);
void clkmeasure_cmd (int , char**);
void clkreset_cmd (int , char**);
void clkphase_cmd (int , char**);
void listdev_cmd (int , char**);
void read_cmd (int , char**);
void write_cmd (int , char**);
void readbase_cmd (int , char**);
void writebase_cmd (int , char**);
void setbase_cmd (int , char**);
void regwrite_cmd (int , char**);
void regread_cmd (int , char**);
void adcreset_cmd (int , char**);
void adcsetreg_cmd (int , char**);
void adcgetphase_cmd (int , char**);
void ifconfig_cmd (int , char**);
void bramwrite_cmd (int , char**);
void bramread_cmd (int , char**);
void bramdump_cmd (int , char**);
static void startudp_cmd(int, char**);
static void endudp_cmd(int, char**);


/* Commands */
/* ********************************* */
static tinysh_cmd_t cmd_version = {0,"version","Displays info about current design","",version_cmd,0,0,0};
static tinysh_cmd_t cmd_1 = {0,"clkmeasure","measure the frequency of the user clock","",clkmeasure_cmd,0,0,0};
static tinysh_cmd_t cmd_2 = {0,"clkreset","reset the DLL on the user clock","",clkreset_cmd,0,0,0};
static tinysh_cmd_t cmd_3 = {0,"clkphase","set the phase of the DLL on the user clock","<phase in degrees>",clkphase_cmd,0,0,0};
static tinysh_cmd_t cmd_4 = {0,"listdev","lists the devices available in the design","",listdev_cmd,0,0,0};
static tinysh_cmd_t cmd_5 = {0,"read","reads words from memory","<start_address> [<end_address>]",read_cmd,0,0,0};
static tinysh_cmd_t cmd_6 = {0,"write","writes data to memory","<access_type (b|s|l)> <address> <data>",write_cmd,0,0,0};
static tinysh_cmd_t cmd_7 = {0,"readb","reads words from memory using base address","<start_address> [<end_address>]",readbase_cmd,0,0,0};
static tinysh_cmd_t cmd_8 = {0,"writeb","writes data to memory using a base address","<access_type> <address> <data>",writebase_cmd,0,0,0};
static tinysh_cmd_t cmd_9 = {0,"setb","sets a base address","<address>",setbase_cmd,0,0,0};
static tinysh_cmd_t cmd_10 = {0,"regwrite","writes the value of a register","<register name> <register value>",regwrite_cmd,0,0,0};
static tinysh_cmd_t cmd_11 = {0,"regread","reads the value of a register","<register name>",regread_cmd,0,0,0};
static tinysh_cmd_t cmd_12 = {0,"adcreset","resets an ADC board","<interleave mode> [<adc>]",adcreset_cmd,0,0,0};
static tinysh_cmd_t cmd_13 = {0,"adcsetreg","sets the value of an ADC register","<adc> <register address> <register value>",adcsetreg_cmd,0,0,0};
static tinysh_cmd_t cmd_14 = {0,"adcgetphase","get the clock phase between the two ADCs","",adcgetphase_cmd,0,0,0};
static tinysh_cmd_t cmd_15 = {0,"ifconfig","Displays network interface configuration","",ifconfig_cmd,0,0,0};
static tinysh_cmd_t cmd_16 = {0,"bramwrite","writes a value in a bram","<bram name> <address> <value>",bramwrite_cmd,0,0,0};
static tinysh_cmd_t cmd_17 = {0,"bramread","reads a value from a bram","<bram name> <address>",bramread_cmd,0,0,0};
static tinysh_cmd_t cmd_18 = {0,"bramdump","dumps the content of a bram","<bram name> [start length]",bramdump_cmd,0,0,0};
static tinysh_cmd_t cmd_19 = {0,"startudp","","",startudp_cmd,0,0,0};
static tinysh_cmd_t cmd_20 = {0,"endudp","","",endudp_cmd,0,0,0};

/* Globals for the udp server */
/* ------------------------------- */
struct udp_pcb *udppcb;
struct ip_addr addr;
struct pbuf *somebuf;
int i=0, j=0, k=0;
int numpackets;
int waittime;
Xuint32 dirxvalue, accnum, curaccnum, loadindicator, bramtodump;
char bootpacket[1500];
int udpswitch = 0;
Xuint8 vectornum;
Xuint32 dumpbram;

void 
display_welcome_msg(void)
{
  /* display welcome message */
  xil_printf("\n\r");
  xil_printf("****************************\n\r");
  xil_printf("* TinySH lightweight shell *\n\r");
  xil_printf("****************************\n\r");

#define DESIGN_NAME "heilesspec_be_nopfb"
#define COMPILED_ON "16-Jul-2008 02:22:20"
	xil_printf("Design name : " DESIGN_NAME "\n\r");
	xil_printf("Compiled on : " COMPILED_ON "\n\r");
}

static void startudp_cmd(int argc, char **argv)
{
	// Usage is startudp <octet1> <octet2> <octet3> <octet4> <port>
	// example: startudp 169 254 128 10 6969
	// will send udp packets to 169.254.128.10 on port 6969


	int udpport=0;
	
	if(argc!=6) {
		xil_printf("Wrong number of arguments\n\r");
		return;
	}
	udpport = tinysh_atoxi(argv[5]);
	xil_printf("Instantiating UDP Server...\r\n");
	xil_printf("Will transmit to IP Address: %d.%d.%d.%d on port: %d\r\n", tinysh_atoxi(argv[1]),tinysh_atoxi(argv[2]),tinysh_atoxi(argv[3]),tinysh_atoxi(argv[4]), udpport);
	IP4_ADDR(&addr, tinysh_atoxi(argv[1]),tinysh_atoxi(argv[2]),tinysh_atoxi(argv[3]),tinysh_atoxi(argv[4])); 
	udppcb = udp_new();		
	somebuf = pbuf_alloc(PBUF_TRANSPORT, 1045, PBUF_ROM);
	//udppcb->flags &= ~(UDP_FLAGS_UDPLITE);
	udp_connect(udppcb, &addr, udpport);
	xil_printf("UDP pcb instantiated\n\r");


/* packet headers */
/* the 21 byte UDP data packets look like this: */
/* 8 bytes (double) time */
/* 1 byte x engine number */
/* 4 bytes (unsigned int) vector number within an integration */
/*   (ie if you want to send 10MB every integration and you break it up into 1MB packets, this */
/*    number will range from 0 to 9) */
/* 4 bytes (unsigned int) flags (not used atm - foresee use to flag bad data) */
/* 4 bytes (unsigned int) data length (in bytes) (payload length) */

dumpbram = XPAR_HEILESSPEC_TWO_INP_POWER_BRAM_MSB_BASEADDR;



	  /* 4 bytes MSBs of 8 bytes representing accumulation number */
	  bootpacket[0] = 0x0;
	  bootpacket[1] = 0x0;
	  bootpacket[2] = 0x0;
	  bootpacket[3] = 0x0;
	
	  /* 3 bytes MSBs of 4 bytes representing vector number */
	  bootpacket[9] = 0x0;
	  bootpacket[10] = 0x0;
	  bootpacket[11] = 0x0;

	  /* 4 bytes representing flags (unused) */
	  bootpacket[13] = 0x0;
	  bootpacket[14] = 0x0;
	  bootpacket[15] = 0x0;
	  bootpacket[16] = 0x0;
	  
	  /* 4 bytes representing payload length */
	  bootpacket[17] = 0x0;
	  bootpacket[18] = 0x0;
	  bootpacket[19] = 0x0;
	  bootpacket[20] = 0x0;

	  (*somebuf).payload = bootpacket;
	  loadindicator = 9999;
	  udpswitch = 1;
	  accnum = sif_reg_read(XPAR_HEILESSPEC_TWO_INP_ACC_NUM_BASEADDR);
	  curaccnum = accnum;
}

static void endudp_cmd(int argc, char **argv)
{
	  //xil_printf("Removing UDP pcb...");
	  udp_remove(udppcb);
	  pbuf_free(somebuf);
	  //xil_printf("Done\n\r");
	  udpswitch = 0;	  
}




static void version_cmd(int argc, char **argv)
{
  xil_printf("Design name : " DESIGN_NAME "\n\r");
  xil_printf("Compiled on : " COMPILED_ON "\n\r");
}

void process_inputs(int feed_tinysh)
{
#ifdef LWIP_ENABLE
#define ml_offset LWIP_TIMER_CYCLES
  static XTime ml_base=ml_offset;
  XTime ml_new;
#endif // LWIP_ENABLE

  // Process packets
  xemacliteif_input(&default_netif);
    
  // If feeding tinysh
  if (feed_tinysh) {

#ifdef LWIP_ENABLE
    // drain input fifo
    char c;
    printmode = 1;
    while(fifo_get(input_fifo,&c,1)) {
      // Telnet command handling
      // TODO Make this work even across packet boundaries
      while(c==255) { // IAC
        // Get command byte
        fifo_get(input_fifo,&c,1);
        switch(c) {
          case 251: // WILL
          case 252: // WON'T
          case 253: // DO
          case 254: // DON'T
            fifo_get(input_fifo,&c,1); // Eat option code
            break;
          case 255:
            // Pass thru escaped IAC
            immediatemode=1;
            tinysh_char_in(c);
            break;
        }
        // Get next input byte
        if(!fifo_get(input_fifo,&c,1)) {
          // If no next byte, set c to flagval and explicitly break out of loop
          c=255;
          break;
        }
      } // End telnet command handling

      // CR handling
      // TODO Make this work even across packet boundaries
      if(c=='\r') {
        // If next byte exists, but is not a NL and is not a NUL
        if(fifo_get(input_fifo,&c,1) && c!='\n' && c!='\0') {
          // Non-telnet CR sequence.
          // Pass in CR, allow other character through
          immediatemode=0;
          tinysh_char_in('\r');
        } else {
          // Eat NL or NUL character following CR (if any)
          c='\r';
        }
      }

      if(c!=255) {
        immediatemode=((c!='\n'&&c!='\r') ? 1 : 0);
        tinysh_char_in(c);
        if(c!=4) {
          send_buf();
        }
      }
    }
    printmode = 0;
#endif // LWIP_ENABLE

    // Process uart fifo 
    while(!XUartLite_mIsReceiveEmpty(XPAR_RS232_UART_1_BASEADDR)) {     
      tinysh_char_in( (Xuint8)XIo_In32(XPAR_RS232_UART_1_BASEADDR + XUL_RX_FIFO_OFFSET));
    }
  } // if feed_tinysh

#ifdef LWIP_ENABLE
  // Handle timers
  XTime_GetTime(&ml_new);
  if ( ml_new >= ml_base ) {
    ml_base = ml_new + ml_offset;
    // Call lwip_tmr() every ml_offset cycles
    lwip_tmr();
  }
#endif //LWIP_ENABLE

    // Sleep?
}

/* Main thread */
/* ********************************* */
int main(void)
{
#ifdef LINUX_ENABLE
  int i;
#endif // LINUX_ENABLE

  /* activate the cache on the PPC */
  XCache_EnableICache(0x00000001);
  XCache_EnableDCache(0x00000001);

#ifdef LINUX_ENABLE
  if(linux) {
    /* wait for greet */
    i = 0;
    do {
      i <<= 8;
      i |= receive_selectmap_byte();
    } while (i != 0x10FFAA55);
  }
#endif // LINUX_ENABLE

  /* display welcome message */
  display_welcome_msg();

  /* add all the commands */
  tinysh_add_command(&cmd_version);
	tinysh_add_command(&cmd_1);
	tinysh_add_command(&cmd_2);
	tinysh_add_command(&cmd_3);
	tinysh_add_command(&cmd_4);
	tinysh_add_command(&cmd_5);
	tinysh_add_command(&cmd_6);
	tinysh_add_command(&cmd_7);
	tinysh_add_command(&cmd_8);
	tinysh_add_command(&cmd_9);
	tinysh_add_command(&cmd_10);
	tinysh_add_command(&cmd_11);
	tinysh_add_command(&cmd_12);
	tinysh_add_command(&cmd_13);
	tinysh_add_command(&cmd_14);
	tinysh_add_command(&cmd_15);
	tinysh_add_command(&cmd_16);
	tinysh_add_command(&cmd_17);
	tinysh_add_command(&cmd_18);
	tinysh_add_command(&cmd_19);
	tinysh_add_command(&cmd_20);

xil_printf("init commands\n");

  /* add user initilization calls */
	adcinit();
xil_printf("adc_init\n");

	lwipinit();
xil_printf("lwip_init\n");


  /* change the prompt */
  tinysh_set_prompt("IBOB % ");
  /* force prompt display by generating empty command */
  tinysh_char_in('\r');

#ifdef LWIP_ENABLE
  // Init timer
  XTime_SetTime(0);
#endif

  /* loop waiting for input */
  while(1) {
    process_inputs(1);


	if(udpswitch == 1) {		

		    accnum = sif_reg_read(XPAR_HEILESSPEC_TWO_INP_ACC_NUM_BASEADDR);			
			loadindicator++;			
            if(accnum == 0){ 
            curaccnum = -1;
			xil_printf("RESETTING- \r\n");
			
			}
			if(loadindicator >= 50000) {
			xil_printf("ERROR! accumulation length is WAY too low - %d\r\n", accnum);
			loadindicator = 0;
			curaccnum = sif_reg_read(XPAR_HEILESSPEC_TWO_INP_ACC_NUM_BASEADDR);
			}
			if (accnum == (curaccnum + 1)) {

					   bootpacket[4] = (accnum >> 0);
					   bootpacket[5] = (accnum >> 8);
					   bootpacket[6] = (accnum >> 16);
					   bootpacket[7] = (accnum >> 24);

    	   				     bootpacket[12] = (Xuint8) 0;
    	   				     
                             dirxvalue = XIo_In32(dumpbram + 32764);
                             memcpy(bootpacket + 21, &dirxvalue, 4);
                             
                             for(i = 0; i < 255; i++) {
                                     dirxvalue = XIo_In32(dumpbram + (i * 4));
                                     memcpy(bootpacket + (i * 4) + 25, &dirxvalue, 4); 
                             } 

                             memcpy(bootpacket + 13, &loadindicator, 4);
                             somebuf->len = (u16_t) 1045;
                             somebuf->tot_len = (u16_t) 1045;
                             udp_send(udppcb, somebuf);                                            
						  
                         for(k=1020;k < 32764;k=k+1024){
    	   				     bootpacket[12] = (Xuint8) (((k-1020)/1024) + 1);
                             for(i = 0; i < 256; i++) {
                                     dirxvalue = XIo_In32(dumpbram + (i * 4) + k);
                                     memcpy(bootpacket + (i * 4) + 21, &dirxvalue, 4); 
                             } 

                             memcpy(bootpacket + 13, &loadindicator, 4);
                             somebuf->len = (u16_t) 1045;
                             somebuf->tot_len = (u16_t) 1045;
                             udp_send(udppcb, somebuf);                                            
                        }
			 	   
			 	   loadindicator = 0; //reset load cntr
			 	   curaccnum = accnum; //Increment curacnum
			}
	 }

  }
}

/* Tinysh support */
/* ********************************* */

/* we must provide these functions to use tinysh
 */
void tinysh_char_out(unsigned char c)
{
  outbyte(c);
}

// outbyte() is used by xil_printf to emit individual bytes.
// Xilinx includes a definition of outbyte that always goes
// to the serial port.  The definition given here overrides that
// thanks to the wonders of the linker search order and order of
// objects listed on the link command line.
#ifndef LINUX_ENABLE
void outbyte(unsigned char c)
{
  switch(printmode) {
#ifdef LWIP_ENABLE
    case 1:
      /* if printmode is 1 (tcp) fill connection state structure */
      bufferbyte(c);         
      break;

    case 2:
      if (c != '\r') {  
        bufferbyte(c);      
      }
      break;
#endif
    default:
      /* output the character on the serial port */
      XUartLite_SendByte(XPAR_RS232_UART_1_BASEADDR, c);
  }
}
#else // LINUX_ENABLE defined
void tinysh_char_out(unsigned char c)
{
  outbyte(c);
}

void outbyte(unsigned char c)
{
  if(!linux) {
    /* output the character on the serial port */
    XUartLite_SendByte(XPAR_RS232_UART_1_BASEADDR, c);
    return;
  }
  /* memorizes the last byte sent */
  last_byte_written = c;

  /* send a "write at location 1" packet to the control FPGA */
  /* Write command */
  send_selectmap_byte(3);
  /* location = 1 */
  send_selectmap_int3(1);
  /* offset = 0 */
  send_selectmap_int4(0);
  /* size = 1 */
  send_selectmap_int4(1);
  /* payload is the character */
  send_selectmap_byte(c);
  /* interrupt */
  intr_selectmap();
}


int inbyte()
{
  unsigned char cmd;
  unsigned int location,size,offset,block_size,transfer_size;
  int i;
  Xuint32 data;
  core current_core;

  if(!linux) {
    /* check if the serial port has anything for us */
    while(XUartLite_mIsReceiveEmpty(XPAR_RS232_UART_1_BASEADDR)) {
      /* check if the other CPU has booted */
      if(XIo_In32(XPAR_OPB_GETSWITCH_0_BASEADDR)) {
        /* we should reset now */
        data = 0x30000000;
        asm("mtdbcr0 %0" :: "r"(data));
      }

    }
    return (Xuint8)XIo_In32(XPAR_RS232_UART_1_BASEADDR + XUL_RX_FIFO_OFFSET);
  }

  /* buffer initialization*/
  unsigned int MAX_WORDS = (MAX_PACKET_SIZE-10)/4;
  unsigned int rd_buffer[MAX_WORDS];

  int buff_loc = 0;

  /* send a "read at location 0" packet to the control FPGA */
  /* Read command */
  send_selectmap_byte(1);
  /* location = 0 */
  send_selectmap_int3(0);
  /* offset = 0 */
  send_selectmap_int4(0);
  /* size = 1 */
  send_selectmap_int4(1);
  /* wait for packets from the control FPGA */
  /* HS: but we need to interrupt control FPGA to get this request */
  intr_selectmap();
  while(1) {
    cmd = receive_selectmap_byte();
    switch(cmd) {
      case 1 : /* read */
        /* get the location */
        location = receive_selectmap_int3();
        /* get the offset */
        offset = receive_selectmap_int4();
        /* get the size */
        size = receive_selectmap_int4();
        /* if location is 0, then output the location table */
        if(location == 0) {
            /* -- future implementation of location table output here -- */
        } else {
          /* check if location is greater than max loc */
          if(location >= NUM_CORES) {
              /* -- future implementation of error handling here -- */
              /* -- error wrong location -- */
          } else {
            current_core = cores[location];
            switch(current_core.type) {
              case xps_dram :
              case xps_framebuffer :
                XIo_Out32(current_core.address, 1);
              case xps_sw_reg :
              case xps_bram :
                if(current_core.type == xps_sw_reg)
                  block_size = 4;
                if(current_core.type == xps_bram)
                  block_size = 4 * tinysh_atoxi(current_core.params);
                if(current_core.type == xps_dram || current_core.type == xps_framebuffer)
                  block_size = 0x40000000;
                if(offset+size>block_size) {
                  size = block_size - offset;
                }
                if(offset>=block_size) {
                    /* -- future implementation of error handling here -- */
                    /* -- wrong seek -- */
                }
                if(current_core.type != xps_dram && current_core.type != xps_framebuffer)
                  offset += current_core.address;
                data = XIo_In32(offset & 0xFFFFFFFC);
                while(size != 0) {
                  if(size > MAX_PACKET_SIZE-10)
                    transfer_size = MAX_PACKET_SIZE-10;
                  else
                    transfer_size = size;
                  size -= transfer_size;

                  /* Send read ack command */
                  send_selectmap_byte(2);
                  /* location */
                  send_selectmap_int3(location);
                  /* size */
                  send_selectmap_int4(transfer_size);
                  /* payload */
                  for(;transfer_size != 0;transfer_size--) {
                    send_selectmap_byte(((unsigned char *) &data)[offset % 4]);
                    offset++;
                    if(offset % 4 == 0)
                      data = XIo_In32(offset);
                  }
                  intr_selectmap();
                }
                if(current_core.type == xps_dram || current_core.type == xps_framebuffer)
                  XIo_Out32(current_core.address, 0);
                break;
              case xps_fifo :
				if(size > MAX_WORDS*4)
                    size = MAX_WORDS*4;
                if(offset!=0) {
                    /* -- future implementation of error handling here -- */
                    /* -- wrong offset -- */
                }
				/* fill read buffer */
                buff_loc = 0;
                while((buff_loc < size/4) & !(XIo_In32(current_core.address + 4) & 1)) {
                    rd_buffer[buff_loc] = XIo_In32(current_core.address);
                    buff_loc++;
                }

                transfer_size = 4*buff_loc;
                /* Send read ack command */
                send_selectmap_byte(2);
                /* location */
                send_selectmap_int3(location);
                /* size */
                send_selectmap_int4(transfer_size);
                /* payload */
                while (buff_loc != 0){
                  send_selectmap_int4(rd_buffer[(transfer_size/4) - buff_loc]);
                  buff_loc--;
                }
                intr_selectmap();
                break;
              default :
                  /* -- future implementation of error handling here -- */
                  /* -- error unsupported location -- */
                break;
            }
          }
        }
        break;
      case 2 : /* read ack */
        /* get the location. It should be 0 only as we don't read from anything else*/
        location = receive_selectmap_int3();
        if(location != 0) xil_printf("Error: Wrong location\n\r");
        /* get the size in bytes. If it is one, it means that one of our read succedded, and we return the payload */
        size = receive_selectmap_int4();
        if(size == 1) return receive_selectmap_byte();
        /* if we received an error code, we send a new request */
        /* HS should wait a bit before stalling cntrl fpga */
        usleep(100000);
        /* send a "read at location 0" packet to the control FPGA */
        /* Read command */
        send_selectmap_byte(1);
        /* location = 0 */
        send_selectmap_int3(0);
        /* offset = 0 */
        send_selectmap_int4(0);
        /* size = 1 */
        send_selectmap_int4(1);
        /* interrupt */
        intr_selectmap();
        break;
      case 3 : /* write */
        /* get the location */
        location = receive_selectmap_int3();
        /* get the offset */
        offset = receive_selectmap_int4();
        /* get the size */
        size = receive_selectmap_int4();
        /* if location is 0, then this is an error */
        if(location == 0) {
            send_writeack(location, -2);
            /* -- future implementation of error handling here -- */
            /* -- error unwritable -- */
        } else {
          /* check if location is greater than max loc */
          if(location >= NUM_CORES) {
              /* -- future implementation of error handling here -- */
              /* -- error wrong location -- */
              send_writeack(location, -3);
          } else {
            current_core = cores[location];
            switch(current_core.type) {
              case xps_dram :
              case xps_framebuffer :
                XIo_Out32(current_core.address, 1);
              case xps_sw_reg :
              case xps_bram :
                if(current_core.type == xps_sw_reg)
                  block_size = 4;
                if(current_core.type == xps_bram)
                  block_size = 4 * tinysh_atoxi(current_core.params);
                if(current_core.type == xps_dram || current_core.type == xps_framebuffer)
                  block_size = 0x40000000;
                if(offset+size>block_size) {
                  size = block_size - offset;
                }
                if(offset>=block_size) {
                    /* -- future implementation of error handling here -- */
                    /* -- wrong seek -- */
                }
                if(current_core.type != xps_dram && current_core.type != xps_framebuffer)
                  offset += current_core.address;
                /* prefetch data for the first read modify write operation */
                data = XIo_In32(offset & 0xFFFFFFFC);
                /* payload processing */
                for(transfer_size = 0;transfer_size < size;transfer_size++) {
                  ((unsigned char *) &data)[offset % 4] = receive_selectmap_byte();
                  offset++;
                  if(offset % 4 == 0) {
                    XIo_Out32(offset-4,data);
                    /* save some cycles */
                    if (size - transfer_size <= 4) {
                        data = XIo_In32(offset & 0xFFFFFFFC);
                    }
                  }
                }
                if (offset % 4 != 0) {
                    XIo_Out32(offset & 0xFFFFFFFC, data);
                }
                /* Send write ack command */
                send_selectmap_byte(4);
                /* location */
                send_selectmap_int3(location);
                /* size */
                send_selectmap_int4(size);
                /* interrupt */
                intr_selectmap();
                if(current_core.type == xps_dram || current_core.type == xps_framebuffer)
                  XIo_Out32(current_core.address, 0);
                break;
              case xps_fifo :
                if(offset!=0) {
                    /* -- future implementation of error handling here -- */
                    /* -- wrong offset -- */
                }
                /* payload processing */
				transfer_size = 0;
				while ((size-transfer_size >=4) & !(XIo_In32(current_core.address + 4) & 1)) {
					XIo_Out32(current_core.address + 0, receive_selectmap_int4());
					transfer_size +=4;
				}
                /* Send write ack command */
                send_selectmap_byte(4);
                /* location */
                send_selectmap_int3(location);
                /* size */
                send_selectmap_int4(transfer_size);
                /* interrupt */
                intr_selectmap();
                break;
              default :
                  /* -- future implementation of error handling here -- */
                  /* -- error unsupported location -- */
                break;
            }
          }
        }
        break;
      case 4 : /* write ack */
        /* get the location */
        location = receive_selectmap_int3();
        /* get the size */
        size = receive_selectmap_int4();
        /* check the size */
        if(size==0) {
          /* last write failed, send it again */
          /* send a "write at location 1" packet to the control FPGA */
          /* Write command */
          send_selectmap_byte(3);
          /* location = 1 */
          send_selectmap_int3(1);
          /* offset = 0 */
          send_selectmap_int4(0);
          /* size = 1 */
          send_selectmap_int4(1);
          /* payload is the character */
          send_selectmap_byte(last_byte_written);
          /* interrupt */
          intr_selectmap();

        }
        break;
      case 17: /* bye */
          /* remove magic from the fifo */
          receive_selectmap_byte();
          receive_selectmap_byte();
          receive_selectmap_byte();
          data = 0x30000000;
          asm("mtdbcr0 %0" :: "r"(data));
        break;
      default : /* unknown packet type */
          /* -- future implementation of error handling here -- */
          /* -- error unknown packet -- */
        break;
    }
  }
}

void send_writeack(unsigned location, int val)
{
    /* Send write ack command */
    send_selectmap_byte(4);
    /* location */
    send_selectmap_int3(location);
    /* size */
    send_selectmap_int4(val);
    /* interrupt */
    intr_selectmap();
}

void intr_selectmap(void)
{
    XIo_Out32(XPAR_OPB_SELECTMAP_FIFO_0_BASEADDR, 1);
}

void send_selectmap_byte(unsigned char c)
{
  int retry = 0;

  /* block on fifo full */
  while (!SELECTMAP_WFIFO_CNT(GET_STATUS())) {
    if (!(retry & 0xFFFF)) {
      intr_selectmap();
    }
    retry += 1;
  }
  /* send the byte */
  XIo_Out8(XPAR_OPB_SELECTMAP_FIFO_0_BASEADDR+4, c);
}

unsigned char receive_selectmap_byte()
{
  Xuint32 data;
  /* block on fifo empty */
  while (!SELECTMAP_RFIFO_CNT(GET_STATUS())) {
    /* check if the other CPU got out of linux */
    if(!XIo_In32(XPAR_OPB_GETSWITCH_0_BASEADDR)) {
      /* we should reset now */
      data = 0x30000000;
      asm("mtdbcr0 %0" :: "r"(data));
    }

  }
  /* read the byte */
  return XIo_In8(XPAR_OPB_SELECTMAP_FIFO_0_BASEADDR+4);
}

unsigned int receive_selectmap_int3()
{
  unsigned int data = 0;

  data = receive_selectmap_byte();
  data <<= 8;
  data |= receive_selectmap_byte();
  data <<= 8;
  data |= receive_selectmap_byte();

  return data;
}

void send_selectmap_int3(unsigned int data)
{
  send_selectmap_byte(data >> 16);
  send_selectmap_byte(data >>  8);
  send_selectmap_byte(data >>  0);
}

unsigned int receive_selectmap_int4()
{
  unsigned int data = 0;

  data = receive_selectmap_byte();
  data <<= 8;
  data |= receive_selectmap_byte();
  data <<= 8;
  data |= receive_selectmap_byte();
  data <<= 8;
  data |= receive_selectmap_byte();

  return data;
}

void send_selectmap_int4(unsigned int data)
{
  send_selectmap_byte(data >> 24);
  send_selectmap_byte(data >> 16);
  send_selectmap_byte(data >>  8);
  send_selectmap_byte(data >>  0);
}
#endif
