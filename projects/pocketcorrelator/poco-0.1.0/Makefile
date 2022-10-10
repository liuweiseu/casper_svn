CC=gcc

all:
	$(CC) -o corrplot src/corrplot.c -l grace_np -lm
	$(CC) -o gen_config_file src/gen_config_file.c
	$(CC) -o poco_conf src/poco_conf.c
	$(CC) -o udprec src/udprec.c

clean:
	rm -rf corrplot gen_config_file poco_conf udprec
	rm -rf poco/c2m.pyc poco/cn_conf.pyc poco/__init__.pyc poco/packet.pyc poco/sim_cn_data.pyc

