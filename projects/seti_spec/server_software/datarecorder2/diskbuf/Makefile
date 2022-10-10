INCS = -I/usr/local/grace/include -I/opt/local/include
LIBS = -lstdc++ -lgrace_np -lgsl -lgslcblas -L/usr/local/grace/lib -L/opt/local/lib

all: default plotter quick

default: test_diskbuf.C
	gcc -lstdc++ -g -o test_diskbuf test_diskbuf.C

plotter: setiread.o readdata.o readdata_continuous.o genplots.o gencsv.o quicklook.o Makefile
	gcc -g -o readdata setiread.o readdata.o ${LIBS} ${INCS}
	gcc -g -o readdata_continuous setiread.o readdata_continuous.o ${LIBS} ${INCS}
	gcc -g -o gencsv setiread.o gencsv.o ${LIBS} ${INCS}
	gcc -g -o quicklook setiread.o quicklook.o ${LIBS} ${INCS}
	gcc -g -o genplots setiread.o genplots.o ${LIBS} ${INCS}

quick:
	gcc -g -o quick_analyzer quick_analyzer.C ${LIBS} ${INCS}

readdata.o: readdata.C Makefile
	gcc -c readdata.C -o readdata.o ${INCS}	
    
readdata_continuous.o: readdata_continuous.C Makefile
	gcc -c readdata_continuous.C -o readdata_continuous.o ${INCS}

gencsv.o: gencsv.C Makefile
	gcc -c gencsv.C -o gencsv.o ${INCS}

quicklook.o: quicklook.C Makefile
	gcc -c quicklook.C -o quicklook.o ${INCS}

genplots.o: genplots.C Makefile
	gcc -c genplots.C -o genplots.o ${INCS}

setiread.o: setiread.C Makefile 
	gcc -c setiread.C -o setiread.o ${INCS}
    
clean:
	@rm -rf *.o	

