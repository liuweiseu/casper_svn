CC=gcc

all:
	mkdir bin
	$(CC) -o bin/corrplot src/corrplot.c -l grace_np -lm
	$(CC) -o bin/gen_config_file src/gen_config_file.c
	$(CC) -o bin/poco_conf src/poco_conf.c
	$(CC) -o bin/udprec src/udprec.c

clean:
	rm -f corrplot gen_config_file poco_conf udprec
	rm -r bin/
	rm -rf poco/c2m.pyc poco/cn_conf.pyc poco/__init__.pyc poco/packet.pyc poco/sim_cn_data.pyc
