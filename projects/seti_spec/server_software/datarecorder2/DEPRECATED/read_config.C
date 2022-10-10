#include <stdio.h>
#include <stdlib.h>
#include <string.h>

double CenterFreq;
int NumDataStreams;

char ConfigBlock[512];
char HeaderBlock[512];
char Header[1024];

void GetConfig(char * ConfigFile);
void WriteHeader(char * HeaderFile);

main(int argc, char ** argv) {

    GetConfig(argv[1]);

    fprintf(stderr, "center_freq = %lf nds = %d\n", CenterFreq, NumDataStreams);

    WriteHeader(argv[2]);

}

void WriteHeader(char * HeaderFile) {

    FILE * f;
    int i, item;
    char * ConfigPoint;

    f = fopen(HeaderFile, "w");

    for(item = 0, i = 0; item < 2; item++, i += strlen(&HeaderBlock[i]) + 1)
        switch(item)
        {
        case 0 : sprintf(&HeaderBlock[i], "%s %lf\n", "CENTER_FREQ= ", CenterFreq);
            break;
        case 1 : sprintf(&HeaderBlock[i], "%s %d\n", "NUM_DATA_STREAMS= ", NumDataStreams);
            break;
        }

    ConfigPoint = Header+512;
    memcpy(&Header, &HeaderBlock, sizeof(HeaderBlock));
    memcpy(ConfigPoint, &ConfigBlock, sizeof(ConfigBlock));

    fwrite(&Header, sizeof(Header), 1, f);

    fclose(f);
}



void GetConfig(char * ConfigFile) {

    FILE * f;
    int i;
    char Buf[512];
    char Keyword[512];
    char Value[512];

    f = fopen(ConfigFile, "r");

    i = 0;
    while (i < sizeof(ConfigBlock)  && fgets(&ConfigBlock[i], sizeof(ConfigBlock)/2, f)) {

        if(ConfigBlock[i] != '#') {
            sscanf(&ConfigBlock[i], "%s %s", Keyword, Value);

            if(strcasecmp(Keyword, "center_freq") == 0) {
                CenterFreq = atof(Value);
            } else if (strcasecmp(Keyword, "num_data_streams") == 0) {
                NumDataStreams = (int)atoi(Value);
            }

            i += strlen(&ConfigBlock[i]) + 1;
        }
    }

    fclose(f);
}
