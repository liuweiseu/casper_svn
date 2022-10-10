#Attach the EDT 016-01607-01 loopback cable between the two DB9 connectors
#before running this test.
#
#Could have multiple PCD-GPSSE's in one host, each has a unit number
#To select a second PCD-GPSSE board, use "-u 1" where this script has "-u 0"

#Load the interface Xilinx (an XC4036XLA) on the main board.
bitload -u 0 ssebio.bit

#Load the XC2S100 Xilinx on the daughterboard
echo  "xlf bitfiles/XC2S100/ssetio.bit"        > sseload.pdb

#Set up the oscillator for the outgoing data on channel B for 50 MHz
#Not actually needed since we give a "-f 50" arg to sserd below
echo  "fosc 50"               >>sseload.pdb

#Write a value of 0x20 to the register address 0x80 in the XC2S100 so firmware
#generates binary 8 bit count for outgoing data, "w 80 00" for DMA writes
echo  "w 80 20"               >>sseload.pdb

#Configure the ECL logic for the default of channel A in, channel B out
echo  "spal 000000"           >>sseload.pdb

#Run the debugger, executing the commands accumulated in the file sseload.pdb
ssepdb -u 0 sseload.pdb

#The outgoing 8 bit count is generated automatically in firmware,
#no need for an outgoing DMA channel.
#Do a DMA read of 100000 bytes
sserd  -u 0  -c 0  -o out  -f 50  100000

#Check to see if the acquired data is 8 bit binary count
#One of the 8 bit alignments reported on should show zero errors
sserot8 out


#Now loop on the above test for all multiples of 10MHz between 60 and 500 MHz
#If loopback cable is older rev 00 version, see trouble at about 270-310 MHz
#The old ssewr_sset.bit firmware for output corupted data around 100-140 MHz.
#Daughterboards older than rev 11 may need resistor change for clean output.
#Clock generator operates to 800 MHz, boards typically fail around 500 MHz.
#This loop is coded specifically for the bash shell under Linix
#It has been unrolled below to better support other shells

#for ((f=60; f<=500; f+=10));  do 
#    echo
#    sserd3 -u 0 -c 0  -o out -f  $f 100000
#    sserot8 out
#done

sserd  -u 0  -c 0  -o out  -f 10  100000
sserot8 out
sserd  -u 0  -c 0  -o out  -f 120  100000
sserot8 out
sserd  -u 0  -c 0  -o out  -f 290  100000
sserot8 out
sserd  -u 0  -c 0  -o out  -f 400  100000
sserot8 out
sserd  -u 0  -c 0  -o out  -f 400  10000000
sserot8 out
