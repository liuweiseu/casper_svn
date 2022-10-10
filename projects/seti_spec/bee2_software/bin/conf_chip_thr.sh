dir=~/bin/
thrproc=$(ps aux | grep bof | grep thr | awk '{print $2}')
config_file="${dir}bee2.config"

#read in parameters from config file
max_hits_per_bin=$(awk '/max_hits_per_bin/ {print $2}' $config_file)
scale_threshold=$(awk '/scale_threshold/ {print $2}' $config_file)

#set default values if parameters are not defined in 
max_hits_per_bin=${max_hits_per_bin:-25}
scale_threshold=${scale_threshold:-96}

echo Setting the maximum number of hits to: $max_hits_per_bin
echo Setting the scale threshold to $scale_threshold

#====THRESHOLDER=====#

#set the max hits for each pfb bin 
write_reg $thrproc thr_comp1_thr_lim $max_hits_per_bin

#set the scaling factor to x/2^5
write_reg $thrproc thr_scale_p1_scale $scale_threshold

#configure 10GbE
cat ${dir}10gbe.conf > /proc/$thrproc/hw/ioreg/rec_ten_GbE
write_reg $thrproc rec_reg_10GbE_destport0 33001
write_reg $thrproc rec_reg_ip 167772161 

#write_reg $thrproc rec_tge_reset 1
#write_reg $thrproc rec_tge_resethh 1
#write_reg $thrproc rec_tge_reset 0 
#write_reg $thrproc rec_tge_resethh 0

