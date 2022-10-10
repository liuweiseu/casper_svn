// Updated 2009.03.24 Glenn Jones  -  Fixed problem with read only registers to avoid seg faults when run not as root
//									User is responsible for keeping track of read only registers, and not trying to write to them
//									TODO: This program has many memory leaks (No use of free() whatsoever)
//									TODO: Any reason not to increase maximum command length?
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h> 
#include <sys/param.h> 
#include <sys/stat.h> 
#include <signal.h>
#include <sys/wait.h>
#include "debug_macros.h"

#define MAXBUFLEN 4096
#define MAXARGS 64      //the maximum number of arguments that can be passed to exec
#define MAXCMDLEN 64    //the maximum size of a command
#define LOADTIME 5     //wait XX seconds after loading a bitstream before trying to read the list of devices
#define SIGKILL 9

//PACKET TYPES:
//=============
#define PROGDEV  10
#define STATUS   20
#define LISTCMD  30
#define LISTBOF  35
#define LISTDEV  40
#define READ     50
#define INDEXEDREAD 51
#define WRITE    60
#define INDEXEDWRITE 61
#define EXEC    70

#define ERR     110
#define REPORT  120
#define AVAILCMND 130
#define AVAILBOF  135
#define AVAILDEV  140
#define DATA    160
#define EXECREPORT 170

#define DEFAULT_LISTCMD_PATH "/usr/bin"       // path of executable commands
#define DEFAULT_BOF_PATH "/boffiles"        // path for executable bof files

//ERRORS:
//=======
#define ERRDIR "ERR Unable to open directory"
#define ERRSOCKFD "ERR Unable to set socket descriptor"
#define ERRSOCKBND "ERR Unable to bind to socket"
#define ERRRX "ERR Unable to receive packet"
#define ERRTX "ERR Unable to send packet"
#define ERRDEC "ERR decoding packet type"
#define ERRNOTCONFIG "ERR device not configured"
#define ERRCONFIG "ERR unable to configure device"
#define ERRFORK "ERR unable to start chid process"
#define ERRKILL "ERR unable to kill child process"
#define ERRNOPROC "ERR child process does not exist"
#define ERRFOPEN "ERR could not open file"
#define ERRREAD "ERR read packet format error"
#define ERRINDEXEDREAD "ERR indexed read packet format error"
#define ERRWRITE "ERR write packet format error"
#define ERRINDEXEDWRITE "ERR indexed write packet format error"
#define ERRBOUNDRY "ERR out of bound read/write request"
#define ERRDEVNOTFOUND "ERR device not found"
#define ERRINDOUTOFBOUNDS "ERR device index out of bounds"
#define ERRFLUSH "ERR flushing data to file"
#define ERREXEC "ERR could not execute command"

extern char *optarg;

int sockfd;
struct sockaddr_in my_addr;    // server's address information
struct sockaddr_in their_addr; // connector's address information
void reply(int num_bytes);
void reply_error(char * errormessage);
int sendall(int sfd, char *buf, int len, const struct sockaddr *to, socklen_t tolen);
int get_int(unsigned char msb, unsigned char msb2, unsigned char lsb2, unsigned char lsb);
char buf[MAXBUFLEN];
void read_from_device(FILE* stream, unsigned int offset, unsigned int trans_size);
void write_to_device(FILE* stream, unsigned int offset, unsigned int trans_size, int bufOffset);


int main(int argc, char ** argv)
{
    socklen_t addr_len;  
    int fd[2]; //used solely to redirect stdin for the purposes of backgrounding
    int num_bytes;
    unsigned int counter;  
    unsigned int dbgcnt;  
    unsigned int counter2;
    DIR           *d;
    struct dirent *dir;
    int pid = -1;
    int exec_pid = -1;
    char *dev_path = NULL;
    char proc[200];
    FILE *stream;
    FILE **p_devfp;  // open all registers
    int *p_devsize;
    char **p_devname;
    unsigned int offset;
    unsigned int trans_size;
    char reg_name[64];
    char reg_qual[128];
    char cmd[MAXCMDLEN]; // how long can this be????
    char dev[64];
    int len_name;
    int num_dev;
    char *exec_argv[MAXARGS];
    int c;
    char *bof_path = DEFAULT_BOF_PATH;
    // only put this in if we want to force the path the EXEC commands are executed from
    //char *exec_path = DEFAULT_EXEC_PATH;
    int fpga_number;
    int port=4950;
    //read in command line args
    while((c = getopt(argc, argv, "b:p:e:f:d:h")) != -1){
        switch(c) {
            // command line path to find bof files
            case 'b':   
                bof_path=optarg;
                debug_fprintf(stderr,"Command line arg bof path: %s\n", bof_path);
                break;
            case 'd':
                dev_path=optarg;
                debug_fprintf(stderr,"Development mode. Will fake registers from: %s\n", dev_path);
            // only put this in if we want to force the path the EXEC commands are executed from
            //case 'e':   
            //    exec_path=optarg;
            //    debug_fprintf(stderr,"Command line arg exec path: %s\n", exec_path);
            //    break;
            case 'f':   
                fpga_number=atoi(optarg);
                debug_fprintf(stderr,"Command line arg FPGA number: %d\n", fpga_number);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            //display help
            case 'h':  
                printf("Usage: %s [-b bof path] [-f FPGA number] [-p port] [-d register path]\n -d will fake registers from a specified location.\n", argv[0]);
                return 0;
        } // end switch option
    } //end while(get options)
    
    
    // obtain a socket descriptor
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {        
        perror(ERRSOCKFD);
        exit(1);
    }
    
    my_addr.sin_family = AF_INET;         // host byte order
    my_addr.sin_port = htons(port);     // short, network byte order
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY); // automatically fill with my IP  ... doesn't matter if we use hton on INASSR_ANY or not
    // since it's value is zero
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);


/*    // lose the pesky "Address already in use" error message
    if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,1,sizeof(int)) == -1) {
	    perror("setsockopt");
	    exit(1);
    } 
*/    
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr) == -1) {
        perror(ERRSOCKBND);
        exit(1);
    }
    
    printf("Listening on IP address %s on port %i\n",inet_ntoa(my_addr.sin_addr),port);
    
    addr_len = sizeof their_addr;
    
    while (1) {
        // read a packet in, exit if there is an error reading
        if ((num_bytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror(ERRRX);
            exit(1);
        }
        
        debug_fprintf(stderr,"Packet from  %s with len %d \n",inet_ntoa(their_addr.sin_addr),num_bytes);
        buf[num_bytes] = '\0';
        debug_fprintf(stderr,"CONTENTS: %s\n", buf);
        
        // process the command 
        switch (buf[0]) {
                //program the FPGA and keep track of the child's process id.
            case    PROGDEV :   
                debug_fprintf(stderr,"PROGDEV:");
                
                //unload the previous design if necessary
                if (pid>0){ 
                    debug_fprintf(stderr,"Closing %i open files...\n",num_dev);
                    for(counter=0; counter<num_dev; counter++)
                    {
						if (p_devfp[counter] != NULL) {			// Make sure fp exists before trying to close it to avoid seg fault
							fclose(p_devfp[counter]);
						}
						else {
							debug_fprintf(stderr,"Did Not close: %d, fp was null",counter)
						}
                    }
                    num_dev=0;
                    
                    debug_fprintf(stderr,"Trying to kill process %i...\n",pid);
                    counter = kill(pid,SIGKILL);
                    if (counter != 0){
                        reply_error(ERRKILL);
                        debug_fprintf(stderr,ERRKILL);
                        if (counter == ESRCH){
                            reply_error(ERRNOPROC);
                            debug_fprintf(stderr,ERRNOPROC);
                        }
                    }
                    wait(NULL);
                    pid = -1;      
                    proc[0] = 0;
                }
                
                //check to see if the user is trying to load a new bof file
                if (num_bytes>2){ 
                    
                    snprintf(cmd, MAXCMDLEN,"%s/%s",bof_path, &buf[1]);
                    
                    debug_fprintf(stderr,"Cmd is: %s with len: %d\n", cmd, strlen(cmd));
                    //Sneakily redirect standard input to prevent stopping when backgrounding 
                    debug_fprintf(stderr,"making a fd...\n");
                    pipe(fd);
                    dup2(fd[0],0);
                    debug_fprintf(stderr,"Forking...\n");
                    pid=fork();                                
                    
                    if (pid < 0){ //failed.
                        debug_fprintf(stderr,ERRFORK);
                        reply_error(ERRFORK);
                    } else if (pid == 0) { //child process
                        if (execlp(cmd , cmd, (char *) 0) < 1) {
                            dup2(fd[0],0);
                            debug_fprintf(stderr,ERRCONFIG);
                            reply_error(ERRCONFIG);
                        }
                        // exit if child could'nt execute the cmd
                        exit(1);
                    } else {    //success, parent process!
                        if (dev_path){
                            sprintf(proc,"%s",dev_path);
                        }
                        else{
                            sprintf(proc,"/proc/%i/hw/ioreg",pid);
                        }
                        debug_fprintf(stderr,"New bitstream: %s with registers in %s\n",cmd,proc);     
                        
                        sleep(LOADTIME); //wait 'till the FPGA is programmed before trying to read registers 

                        // check that we can open the directory ok
                        if ((d = opendir(proc))==NULL){ 
                            debug_fprintf(stderr,ERRDIR);  
                            kill(pid,SIGKILL);
                            pid = -1;
                            proc[0]=0;
                            break;
                        }
                        rewinddir(d);
                            
                        // generate dev_table here, every time FPGA is programed
                        counter =0;
                        while ( (dir = readdir(d)) ) 
                        {
                            if ( strcmp( dir->d_name, ".") == 0 || strcmp( dir->d_name, "..") ==0){
                                debug_fprintf(stderr,"Ignoring . or ..\n");
                                continue;
                            }
                            else {
                                debug_fprintf(stderr,"found file: %d: %s\n",counter, dir->d_name);
                                ++counter;
                            }
                        }
                        rewinddir(d);
                        num_dev = counter; 
                        
                        debug_fprintf(stderr,"number of devs in proc directory: %d\n", counter);
                        
                        p_devfp = (FILE **)malloc(num_dev*(sizeof(FILE *)));
                        p_devsize = (int *)malloc(num_dev*(sizeof(int)));
                        p_devname = (char **)malloc(num_dev*(sizeof(char *)));
                        
                        
                        counter = 0;
                        while ( (dir = readdir(d)) ) {
                            // copy the file name so long as it isn't '.' or '..'
                            if ( !(strcmp( dir->d_name, ".")==0 || strcmp( dir->d_name, "..") ==0) )
                            {
                                // dev name indexes
                                p_devname[counter] = (char *)malloc(strlen(dir->d_name)+1);
                                strcpy( p_devname[counter], dir->d_name);               
                                // dev file indexes
                                sprintf( dev, "%s/%s", proc, p_devname[counter]);
                                debug_fprintf(stderr,"Opening %i: %s...", counter,dev);
                                if ((p_devfp[counter] = fopen(dev,"r+")) != NULL) {
                                    debug_fprintf(stderr, "Success!");
                                    //seek to the end to get the size of the file
                                    fseek(p_devfp[counter], 0L, SEEK_END);
                                    p_devsize[counter] = ftell(p_devfp[counter]);
                                    debug_fprintf(stderr," Size: %d\n", p_devsize[counter]);  
                                }
								else if ((p_devfp[counter] = fopen(dev,"r")) != NULL) {
                                    debug_fprintf(stderr, "Success opening as Read-Only");
                                    //seek to the end to get the size of the file
                                    fseek(p_devfp[counter], 0L, SEEK_END);
                                    p_devsize[counter] = ftell(p_devfp[counter]);
                                    debug_fprintf(stderr," Size: %d\n", p_devsize[counter]);  
                                }
                                else{
                                    debug_fprintf(stderr,"ERR opening p_devfp[%i]:%s\n",counter,p_devname[counter]);
                                }
                                ++counter;                                      
                            } 
                        } // end while we're still getting a list of files and their sizes
                    }
                } //end if we're trying to program the FPGA
                break;
                
                case    STATUS :    //return a string representing the filesystem location of the process, or error if FPGA is not yet configured.
                debug_fprintf(stderr,"STATUS:\n");
                //check if FPGA is configured
                if (pid < 0){
                    reply_error(ERRNOTCONFIG);
                    debug_fprintf(stderr,ERRNOTCONFIG);
                } else{
                    buf[0] = REPORT;
                    for (counter=0;counter<strlen(proc);counter++){ 
                        buf[counter+2] = proc[counter];
                    }
                    reply(counter+2);
                    debug_fprintf(stderr,"%s\n",proc);
                }
                break;
                
                case    LISTCMD :   //Iterate through each item in the CMD_PATH directory and add it to the buf buffer, along with its length
                debug_fprintf(stderr,"LISTCMD\n"); 
                buf[0] = AVAILCMND; //give the reply packet a type. Tag id remains as it was in buf[1]
                counter = 2;
                if ((d = opendir(DEFAULT_LISTCMD_PATH)) == NULL){
                    //perror(ERRDIR);
                    debug_fprintf(stderr,ERRDIR);
                    reply_error(ERRDIR);
                    
                } else {
                    while( ( dir = readdir( d ) ) ) {
                        if( strcmp( dir->d_name,"." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) {
                            continue;
                        } else strcpy( &buf[counter], dir->d_name);
                        
                        counter += (strlen(dir->d_name)+1);
                        debug_fprintf(stderr,"LISTCMD: dir->d_name *: %s\n", dir->d_name);
                        
                    } //end while still stuff in the directory
                } //end else if the directory does not exist
                closedir( d );  
                
                debug_fprintf(stderr,"LISTCMD: sending %d bytes.\n", counter); 
                reply(counter);
                break;
                
                case    LISTBOF :   //Iterate through each item in the BOF_PATH directory and add it to the buf buffer, along with its length
                debug_fprintf(stderr,"LISTBOF\n"); 
                buf[0] = AVAILBOF; //give the reply packet a type. Tag id remains as it was in buf[1]
                counter = 2;
                if ((d = opendir(DEFAULT_BOF_PATH)) == NULL){
                    debug_fprintf(stderr,ERRDIR);
                    reply_error(ERRDIR);
                    
                } else {
                    while( ( dir = readdir( d ) ) ) {
                        if( strcmp( dir->d_name,"." ) == 0 || strcmp( dir->d_name, ".." ) == 0 ) {
                            continue;
                        } else strcpy( &buf[counter], dir->d_name);
                        
                        counter += (strlen(dir->d_name)+1);
                        debug_fprintf(stderr,"LISTBOF: dir->d_name *: %s\n", dir->d_name);
                        
                    } //end while still stuff in the directory
                } //end else if the directory does not exist
                closedir( d );  
                
                debug_fprintf(stderr,"LISTBOF: sending %d bytes.\n", counter); 
                reply(counter);
                break;
                
                case    LISTDEV :   
                debug_fprintf(stderr,"LISTDEV\n");
                if (pid > 0){ 
                    buf[0] = AVAILDEV; //give the reply packet a type. Tag id remains as it was in buf[1]
                    debug_fprintf(stderr,"PID IS %i. with %i devices.\n",pid, num_dev);
                    counter = 2;      
                    for (counter2 = 0; counter2 < num_dev; counter2++) {
                        debug_fprintf(stderr,"LISTDEV: buffering %s\n",p_devname[counter2]);
                        strcpy(&buf[counter], p_devname[counter2]);
                        counter += (strlen(p_devname[counter2])+1);
                    }
                    
                    debug_fprintf(stderr,"LISTDEV: num bytes transmited: %d\n", counter);
                    reply(counter); 
                } else {
                    reply_error(ERRNOTCONFIG);
                }
                break;
                
                case    READ :      //read a user-specified file
                debug_fprintf(stderr,"READ:\n");
                if (pid>0) {
                    //parse the request:
                    if (num_bytes > 10){
                        offset = get_int(buf[2],buf[3],buf[4],buf[5]);
                        trans_size = get_int(buf[6],buf[7],buf[8],buf[9]);
                        
                        for (counter=10;counter<num_bytes; counter++){
                            reg_name[counter-10] = buf[counter];
                            //printf("counter: %i, char: %c\n",counter,reg_name[counter-10]);
                        }
                        reg_name[counter-10] = 0;    
                        
                        sprintf(reg_qual,"%s/%s",proc,reg_name);
                        debug_fprintf(stderr,"READ: looking up file %s from index %i for %i bytes.\n",reg_qual,offset,trans_size);
                        
                        stream = fopen(reg_qual,"r");
                        if (stream !=NULL){	  
                            debug_fprintf(stderr,"Success openning the file!\n");
                            read_from_device(stream, offset, trans_size);
                            fclose(stream);
                        } 
                        else {
                            reply_error(ERRFOPEN);
                            debug_perror(ERRFOPEN);
                        }
                    }  // if we got at least 12 bytes, register_name at least of length 2 (null terminated)
                    else {   
                        reply_error(ERRREAD);
                        //perror(ERRREAD);
                    }    
                } else { //the device isn't programmed yet (pid<=0)
                    reply_error(ERRNOTCONFIG);
                }
                break;            
                
                //read from the device using an index
                case    INDEXEDREAD:
                debug_fprintf(stderr,"INDEXEDREAD:\n");
                if (pid>0) {
                    //parse the request:
                    if (num_bytes == 11){
                        // need to change get_int definition for netcat testing ******************
                        offset = get_int(buf[2],buf[3],buf[4],buf[5]);
                        trans_size = get_int(buf[6],buf[7],buf[8],buf[9]);
                        for (counter=0;counter<num_bytes;counter++){
                            debug_fprintf(stderr,"counter: %i, ASCI: %i char: %c\n",counter,buf[counter],buf[counter]);
                        }    
                        
                        int dev_index = (int) buf[10];
                        
                        //if the register is found, write to it
                        if(dev_index < num_dev)
                        {
                            read_from_device(p_devfp[dev_index], offset, trans_size);
                        }
                        else{
                            reply_error(ERRINDOUTOFBOUNDS);
                            debug_fprintf(stderr,ERRINDOUTOFBOUNDS);
                        }
                        
                    }  // if we got at least 11 bytes, register_name at least of length 2 (null terminated)
                    else {   
                        reply_error(ERRINDEXEDREAD);
                        debug_fprintf(stderr,ERRINDEXEDREAD);
                    }    
                } 
                //the device isn't programmed yet (pid<=0)
                else { 
                    reply_error(ERRNOTCONFIG);
                }
                break;
                
                case    WRITE :     //write to a register
                debug_fprintf(stderr,"WRITE:");  
                if (pid>0) {
                    //parse the request:
                    if (num_bytes > 11){
                        offset = get_int(buf[2],buf[3],buf[4],buf[5]);
                        trans_size = get_int(buf[6],buf[7],buf[8],buf[9]);
                        
                        debug_fprintf(stderr,"offset: %u\n", offset);
                        debug_fprintf(stderr,"trans_size: %u\n", trans_size);
                        
                        counter = 0;
                        
                        while (counter <63) {
                            if ( (reg_name[counter]=buf[counter+10]) == 0 ) break;
                            ++counter;
                        }
                        
                        // for safety
                        reg_name[counter]='\0';
                        
                        sprintf(reg_qual,"%s/%s",proc,reg_name);
                        debug_fprintf(stderr,"looking up file %s from index %i for %i bytes\n",reg_qual,offset,trans_size);
                        
                        
                        // get the length of the register name including the null termination
                        len_name = strlen(reg_name)+1;
                        debug_fprintf(stderr,"reg_name: %s\n", reg_name);
                        debug_fprintf(stderr,"len_name: %d\n", len_name);
                        
                        //search for reg name in table
                        for(counter=0; (counter<num_dev) && (strcmp(p_devname[counter],reg_name) != 0) ; counter++);
                        
                        //if the register is found, write to it
                        if(counter < num_dev)
                        {
                            debug_fprintf(stderr, "file pointer found!\n");
                            write_to_device(p_devfp[counter], offset, trans_size, 10+len_name);                     
                            
                        } else{
                            reply_error(ERRDEVNOTFOUND);
                            debug_fprintf(stderr,ERRDEVNOTFOUND);
                        }
                    } //if we got at least 11 bytes since  len(reg_name), reg_name and Data need to be at least 1 byte. 
                    else {
                        reply_error(ERRWRITE);
                        debug_perror(ERRWRITE);
                    }
                } 
                //the device isn't programmed yet (pid<=0)
                else { 
                    reply_error(ERRNOTCONFIG);
                }
                break;              
                
                case    INDEXEDWRITE:
                debug_fprintf(stderr,"INDEXEDWRITE:");  
                if (pid>0) {
                    //parse the request:
                    if (num_bytes > 11){
                        offset = get_int(buf[2],buf[3],buf[4],buf[5]);
                        trans_size = get_int(buf[6],buf[7],buf[8],buf[9]);
                        
                        debug_fprintf(stderr,"offset: %u\n", offset);
                        debug_fprintf(stderr,"trans_size: %u\n", trans_size);
                        
                        int dev_index = (int) buf[10];
                        
                        //if the register is found, write to it
                        if(dev_index < num_dev)
                        {
                            debug_fprintf(stderr, "file pointer found!\n");
                            write_to_device(p_devfp[dev_index], offset, trans_size, 11);                     
                            
                        } 
                        else{
                            reply_error(ERRINDOUTOFBOUNDS);
                            debug_fprintf(stderr,ERRINDOUTOFBOUNDS);
                        }
                    } //if we got at least 11 bytes since Data needs to be at least 1 byte. 
                    else {
                        reply_error(ERRINDEXEDWRITE);
                        debug_perror(ERRINDEXEDWRITE);
                    }      
                    break;                          
                    
                    default :        
                    debug_fprintf(stderr,ERRDEC);
                    reply_error(ERRDEC);
                } 
                //the device isn't programmed yet (pid<=0)
                else { 
                    reply_error(ERRNOTCONFIG);
                }
                break;
                
                case        EXEC:

                // get the command and arguments: 
                exec_argv[0] = strtok(&(buf[2])," ");
                exec_argv[1] = exec_argv[0];
                for (counter=1; counter<MAXARGS-1 && exec_argv[counter-1]!=NULL; counter++){
                    exec_argv[counter] = strtok(NULL," ");
                    //debug_fprintf(stderr,"Argument %i is: %s\n",dbgcnt,exec_argv[counter]);
                }
                exec_argv[counter] = NULL;
                
                debug_fprintf(stderr,"cmd is: %s\n", exec_argv[0]);
                for (dbgcnt=1; dbgcnt<counter; dbgcnt++){
                    debug_fprintf(stderr,"Argument %i is: %s\n",dbgcnt,exec_argv[dbgcnt]);
                }
                debug_fprintf(stderr,"Command length: %d\n", (int) strlen(cmd));
                debug_fprintf(stderr,"forking!\n");

                exec_pid=fork();  
                if (exec_pid < 0){ //failed
                    debug_fprintf(stderr,ERRFORK);
                    reply_error(ERRFORK);
                } 
                else if (exec_pid == 0) { //child process
                    if (execvp(exec_argv[0] , exec_argv) < 1) {
                        debug_fprintf(stderr,ERREXEC);
                        reply_error(ERREXEC);
                    }
                    // exit if child couldn't execute the cmd
                    debug_fprintf(stderr,"EXEC didn't work... killing child process\n");
                    exit(1);
                }
                else { //parent reply with a success packet
                    buf[0] = EXECREPORT;
                    buf[2] = exec_pid;
                    reply(4);
                    debug_fprintf(stderr,"EXEC: started pid %i\n",exec_pid);
                }
                
                break;
        } //end switch                                 
    } //end while(1)
    
    
    close(sockfd);
    
    return 0;
}

/*
 // for testing purposes and to be able to use decimal numbers
 int get_int(unsigned char msb, unsigned char msb2, unsigned char lsb2, unsigned char lsb) {
 return ((msb-48)*1000)+((msb2-48)*100)+((lsb2-48)*10)+lsb-48;
 }
 */

int get_int(unsigned char msb, unsigned char msb2, unsigned char lsb2, unsigned  char lsb){
    return ((msb<<24)+(msb2<<16)+(lsb2<<8)+lsb);
}


void reply(int num_bytes){
    //int_bytes indicates how many bytes should be transmitted
    if ((sendall(sockfd, buf, num_bytes, (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        //perror(ERRTX);
        exit(1);
    }  
}

void reply_error(char * errormessage){
    int count;
    //int_bytes indicates how many bytes should be transmitted
    buf[0] = ERR;
    for (count=0;count<strlen(errormessage);count++)
        buf[2+count] = errormessage[count];
    if ((sendall(sockfd, buf, count+2,(struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        //perror(ERRTX);
        exit(1);
    }  
}

int sendall(int sfd, char *buf, int len, const struct sockaddr *to, socklen_t tolen)
{
    int total = 0;        // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;
    while(total < len) {
        n = sendto(sfd, buf+total, bytesleft, 0, to, tolen);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }
    //len = total; // return number actually sent here
    return n==-1?-1:0; // return -1 on failure, 0 on success
}

void read_from_device(FILE* stream, unsigned int offset, unsigned int trans_size)
{
    unsigned int endPos;
    unsigned int counter;
    fseek(stream,0L, SEEK_END);
    endPos = ftell(stream);
    debug_fprintf(stderr,"endPos: %u\n",endPos);
    
    if (endPos >= (offset+trans_size)) {
        fseek(stream, offset, SEEK_SET);
        
        buf[0] = DATA;  //give the reply packet type. Tag id remains as it was in buf[1] 
        
        for(counter=0; counter<trans_size; counter++)
            buf[counter+2] = fgetc(stream);
        for(counter=0; counter < 2+trans_size ; counter++)
            debug_fprintf(stderr,"buf[%i]: %c\n",counter,buf[counter]);
        
        // 1+1+trans_size
        reply(2+trans_size);
        
    }	    
    else {
        reply_error(ERRBOUNDRY);
        debug_perror(ERRBOUNDRY);
    }
}

void write_to_device(FILE* stream, unsigned int offset, unsigned int trans_size, int bufOffset)
{
    unsigned int endPos;
    unsigned int counter;
    
    // need to check if the size of the file is large enough
    fseek(stream, 0L, SEEK_END);
    endPos = ftell(stream);
    debug_fprintf(stderr,"endPos: %u\n",endPos);
    
    if (endPos >= (offset+trans_size)) {
        fseek(stream, offset, SEEK_SET);
        
        for (counter = 0; counter<trans_size; counter++) {
            fputc(buf[bufOffset+counter], stream);
            debug_fprintf(stderr,"***: %c\n", buf[bufOffset+counter]);
        }
        //flush any buffered data
        if(fflush(stream) != 0)
        {
            perror(ERRFLUSH);
            reply_error(ERRFLUSH);
        }
    }
    else {
        reply_error(ERRBOUNDRY);
        debug_perror(ERRBOUNDRY);
    }
    
}
