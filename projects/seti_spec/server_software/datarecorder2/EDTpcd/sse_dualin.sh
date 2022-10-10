#Will set up two DMA channels to read two streams of data simultaneously
#Need both connectors hooked up to incomming data streams
#
#Could have multiple PCD-GPSSE's in one host, each has a unit number
#To select a second PCD-GPSSE board, use "-u 1" where this script has "-u 0"

#Load the interface Xilinx (an XC4036XLA) on the main board.
bitload -u 0 ssebii.bit

#Load the XC2S100 Xilinx on the daughterboard
echo  "xlf bitfiles/XC2S100/ssetii.bit"    > sseload.pdb

#Set up the oscillator for the outgoing data on channel B for 50 MHz
#Not actually needed since we give a "-f 50" arg to sserd below
echo  "fosc 50"               >>sseload.pdb

#Write a value of 0x00 to the register address 0x80 for DMA writes
#This is the primary difference between this and sse_loopcount.sh
echo  "w 80 00"               >>sseload.pdb

#Configure the ECL logic for both channels in
echo  "spal 011000"           >>sseload.pdb

#Run the debugger, executing the commands accumulated in the file sseload.pdb
ssepdb -u 0 sseload.pdb



#Read from first channel and check the data.  Assumes data is binary count
#Note that default channel for -c option is 0 if not specified
sserd  -u 0  -c 0  -o out  100000
sserot8 out


#Read from second channel and check the data.  Assumes data is binary count
sserd  -u 0  -c 1  -o out  100000
sserot8 out


echo  "After script starts looping on DMA reads of channel A,"
echo   "initiate DMA read of channel B in another console"
echo  "    sserd  -u 0  -c 1  -o out  100000"
echo  "    sserot8 out"

#Loop forever reading from channel A.
#Channel A has priority, so B looses out if PCI bus bandwidth exceeded.
sserd  -u 0  -c 0  -L 1  100000

