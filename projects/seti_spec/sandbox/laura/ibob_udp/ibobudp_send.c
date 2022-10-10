/********
 * lspitler - 24 Apr 09
 * Sends single command to iBOB 
 * running UDP server.
 * ARG 1 = command
 * *********/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
	char reqpack[1000];
	char recvpack[1000];
	int s, c, b, sb, l, n;
	int len_msg,i;
	struct sockaddr_in sock_loc, sock_rem;
	struct sockaddr_storage new_addr;

/*Prepare request packet*/
	
	for(i = 0; i < 1000; i++)
	{
		reqpack[i]=0;
		recvpack[i]=0;
	}

	len_msg = sprintf(reqpack, "%c%c%c%c%c%c%c%c%s \n", '0' ,'0','0','0','0','0','0','0',argv[1]);
	printf("Length of message: %d\n", len_msg);	

	reqpack[0] = 255;
	reqpack[1] = 0;
	reqpack[2] = 0;
	reqpack[3] = 1;
	reqpack[4] = 0;
	reqpack[5] = 0;
	reqpack[6] = 0;
	reqpack[7] = 255;

/* Open socket */

        s = socket(PF_INET, SOCK_DGRAM, 0);     //SOCK_DGRAM for UDP
                if(s < 0)
                        printf("A socket is not open.\n");

/* Define protocol, port, and IP address */
        sock_rem.sin_family = AF_INET; //Remote socket
        sock_rem.sin_port = htons(7);
        inet_pton(AF_INET, "192.168.1.6", &(sock_rem.sin_addr)); //Set iBOB IP

        sock_loc.sin_family = AF_INET; //Local socket
        sock_loc.sin_port = htons(14850);
        inet_pton(AF_INET, "192.168.1.101", &(sock_loc.sin_addr)); //Set iBOB IP

/* Bind (Isn't needed to send. Sets the receive port. */
	b = bind(s, (struct sockaddr *)&sock_loc, sizeof(sock_loc));	
		if(b<0)
			printf("Bind failed.\n");
/*Connect */
       c = connect(s, (struct sockaddr *)&sock_rem, sizeof(sock_rem) );
                if(c<0)
                        printf("Connect failed.\n");

/* Send */
	sb = send(s, reqpack, len_msg*sizeof(char),0);
		printf("Num bytes sent: %d\n", sb);
	
/*Close socket*/        
        close(s);

}
