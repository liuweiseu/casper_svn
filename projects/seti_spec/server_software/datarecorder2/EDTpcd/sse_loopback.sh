#Attach the EDT 016-01607-01 loopback cable between the two DB9 connectors
#before running this test.
#
#Could have multiple PCD-GPSSE's in one host, each has a unit number
#To select a second PCD-GPSSE board, use "-u 1" where this script has "-u 0"

#Load the interface Xilinx (an XC4036XLA) on the main board.
bitload -u 0 ssebio.bit

#Load the XC2S100 Xilinx on the daughterboard
echo  "xlf bitfiles/XC2S100/ssetio.bit"    > sseload.pdb

#Set up the oscillator for the outgoing data on channel B for 50 MHz
#Not actually needed since we give a "-f 50" arg to sserd below
echo  "fosc 50"               >>sseload.pdb

#Write a value of 0x00 to the register address 0x80 for DMA writes
#This is the primary difference between this and sse_loopcount.sh
echo  "w 80 00"               >>sseload.pdb

#When no outgoing data is available from the host DMA stream, 
#repeatedly send the following byte instead.
echo  "w 82 5a"               >>sseload.pdb

#Configure the ECL logic for the default of channel A in, channel B out
echo  "spal 000000"           >>sseload.pdb

#Run the debugger, executing the commands accumulated in the file sseload.pdb
ssepdb -u 0 sseload.pdb

#Create file ssedata.raw of binary data
ssedata


echo  "After DMA writes have started, initiate a DMA read in another console"
echo  "    sserd  -u 0  -o out  100000"
echo  "    sserot8 out"



#Loop forever writing the file out to channel B
ssewr -u 0 -L ssedata.raw

#We can change the frequency of the outgoing clock using the -f option
#to either sserd.c or ssewr.c
#In this case, use -f with sserd to determine at what frequency the multiple
#DMA channels start to fail, was good to about 210 MHz on my machine.
#
#    sserd  -u 0  -f 100  -o out  100000"
#    sserot8 out"
#    sserd  -u 0  -f 150  -o out  100000"
#    sserot8 out"

