all: Makefile debug_macros.h corr_packet.h simple_packet_counter.c
	gcc -Wall -o simple_packet_counter simple_packet_counter.c

debug: Makefile debug_macros.h corr_packet.h simple_packet_counter.c
	gcc -Wall -D DEBUG -o simple_packet_counter simple_packet_counter.c

clean:
	@rm -vf simple_packet_counter
	@rm -vf *~
