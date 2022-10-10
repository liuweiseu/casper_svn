all: pls_fix pls_skim pcap_split

fix_pls: pls_fix.c
	gcc -g -Wall -o $@ $@.c

skim_pls: pls_skim.c
	gcc -g -Wall -o $@ $@.c

pcap_split: pcap_split.c
	gcc -g -Wall -o $@ $@.c -lpcap

clean:
	@rm -rfv pls_fix
	@rm -rfv pls_skim
	@rm -rfv pcap_split
	@rm -rfv *~

