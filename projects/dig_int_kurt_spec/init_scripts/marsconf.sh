write_reg $1 reg_sync_period 131072 
write_reg $1 reg_acclen 127 
write_reg $1 reg_coeff_stokes1 12288  
write_reg $1 reg_coeff_stokes2 12288
write_reg $1 reg_coeff_stokes3 12288
write_reg $1 reg_coeff_stokes4 12288
write_reg $1 kurt_scale_1a 12288
write_reg $1 kurt_scale_1b 12288
write_reg $1 kurt_output1a 1
write_reg $1 kurt_output1b 1
write_reg $1 reg_output_bitselect1 2
write_reg $1 reg_output_bitselect2 2
write_reg $1 reg_output_bitselect3 2
write_reg $1 reg_output_bitselect4 2

#set packet size
write_reg $1 Packet_Size 1025

#set output dest. IP and port
write_reg $1 reg_ip 167772164
write_reg $1 reg_10GbE_destport0 6001

#configure 10GbE
cat ./10gbe.conf > /proc/$1/hw/ioreg/ten_GbE0

#stuff below for old ibob design
#adc mode
#./ibobudp_tinyshell "write l xd0000000 xffffffff"

#base address
#./ibobudp_tinyshell "setb x40000000"

#sender mac octets 1,2
#./ibobudp_tinyshell "writeb l 0 x00000060"

#sender mac octets 3,4,5,6
#./ibobudp_tinyshell "writeb l 4 xdd47e302"

#gateway ip
#./ibobudp_tinyshell "writeb l 8 x00000000"

#source ip
#./ibobudp_tinyshell "writeb l 12 x0a000001"

#source port
#./ibobudp_tinyshell "writeb b x16 x0f"

#source port
#./ibobudp_tinyshell "writeb b x17 xa0" 

#destination mac 1,2
#./ibobudp_tinyshell "writeb l x3020 x00000060"

#destination mac octets 3,4,5,6
#./ibobudp_tinyshell "writeb l x3024 xdd47e34d"

#finish up
#./ibobudp_tinyshell "writeb b x15 xff" 

#adc mode
#./ibobudp_tinyshell "write l xd0000000 x0"



echo "reading back reg_sync_period"
read_reg $1 reg_sync_period
echo "reading back reg_acclen"
read_reg $1 reg_acclen

echo "reading back reg_coeff_stokes1"
read_reg $1 reg_coeff_stokes1
echo "reading back reg_coeff_stokes2"
read_reg $1 reg_coeff_stokes2
echo "reading back reg_coeff_stokes3"
read_reg $1 reg_coeff_stokes3
echo "reading back reg_coeff_stokes4"
read_reg $1 reg_coeff_stokes4

echo "reading back kurt_scale_1a"
read_reg $1 kurt_scale_1a
echo "reading back kurt_scale_1b"
read_reg $1 kurt_scale_1b

echo "reading back kurt_select1a"
echo "reading back kurt_select1b"

echo "reading back kurt_output1a"
read_reg $1 kurt_output1a
echo "reading back kurt_output1b"
read_reg $1 kurt_output1b

echo "reading back reg_output_bitselect1"
read_reg $1 reg_output_bitselect1
echo "reading back reg_output_bitselect2"
read_reg $1 reg_output_bitselect2
echo "reading back reg_output_bitselect3"
read_reg $1 reg_output_bitselect3
echo "reading back reg_output_bitselect4"
read_reg $1 reg_output_bitselect4

echo "reading back Packet_Size"
read_reg $1 Packet_Size

echo "reading back reg_ip"
read_reg $1 reg_ip
echo "reading back reg_10GbE_destport0"
read_reg $1 reg_10GbE_destport0

