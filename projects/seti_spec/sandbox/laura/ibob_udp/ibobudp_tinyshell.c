/********
 * lspitler - 29 Apr 09
 * Sends (and receives)  single command to iBOB running UDP server.
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
	char recvpack[1032];
	int s, c, b, sb, l, n;
	int len_msg,i;
	int packtype = 1; //Must be one to enter receive loop
	struct sockaddr_in sock_rem;

/*Prepare request packet*/
	
	for(i = 0; i < 1000; i++)
	{
		reqpack[i]=0;
		recvpack[i]=0;
	}

	len_msg = sprintf(reqpack, "%c%c%c%c%c%c%c%c%s \n",'0','0','0','0','0','0','0','0',argv[1]);

	reqpack[0] = 255; //Set first byte of request packet header

/* BEGIN UDP communication */
/* Define protocol, port, and IP address */
        sock_rem.sin_family = AF_INET; //Remote socket
        sock_rem.sin_port = htons(7);
        inet_pton(AF_INET, "192.168.1.6", &(sock_rem.sin_addr)); //Set iBOB IP

/* Open socket */
        s = socket(PF_INET, SOCK_DGRAM, 0);     //SOCK_DGRAM for UDP
                if(s < 0) printf("A socket is not open.\n");
/*Connect */
       c = connect(s, (struct sockaddr *)&sock_rem, sizeof(sock_rem) );
                if(c<0) printf("Connect failed.\n");
/* Send */
	sb = send(s, reqpack, len_msg*sizeof(char),0);

/* Receive loop*/
	while(packtype == 1) //If 1, more packets coming; if 2, last/only
	{
		sb = recv(s, recvpack, 1032*sizeof(char), 0);

		packtype = (int) recvpack[6]; //Read type from header

		for(i = 0 ; i < (sb - 8); i++)  //Rotate bytes to clear header
			recvpack[i] = recvpack[i+8];
		recvpack[sb-8] = 0; //Append null char
		
		printf("%s", recvpack);
	}

/*Close socket*/        
        close(s);

}
