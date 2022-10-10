write_reg $1 xauisnap_ctrl 0
write_reg $1 xauisnap_ctrl 5
write_reg $1 xauisnap_ctrl 3
read_bram_8bit 726 0 2047 xauisnap_bram 
