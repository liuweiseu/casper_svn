#ifndef CORR_PACKET
#define CORR_PACKET

#define HEADER_SIZE 24
//offsets of correlator packet fields
#define TIME_OFFSET 0
#define X_ENGINE_OFFSET 8
#define DATA_OFFSET_OFFSET 12
#define FLAGS_OFFSET 16
#define DATA_LENGTH_OFFSET 20
#define DATA_OFFSET 24

//struct to capture the data from a single packet
typedef struct{
    uint64_t time;
    uint32_t x_engine;
    uint32_t data_offset;
    uint32_t flags;
    uint32_t data_length;
    void *data;
} corr_packet;

//this may need to be altered to correct network/host byte ordering difference
inline void get_time(corr_packet pkt, char *data)
{
    pkt.time = (uint64_t) data[TIME_OFFSET];
}
inline void get_x_engines(corr_packet pkt, char *data)
{
    pkt.x_engine = (uint32_t) data[X_ENGINE_OFFSET];
}
inline void get_data_offset(corr_packet pkt, char *data)
{
    pkt.data_offset = (uint32_t) data[DATA_OFFSET_OFFSET];
}
inline void get_flags(corr_packet pkt, char *data)
{
    pkt.flags = (uint32_t) data[FLAGS_OFFSET];
}
inline void get_data_length(corr_packet pkt, char *data)
{
    pkt.data_length = (uint32_t) data[DATA_LENGTH_OFFSET];
}
inline void get_data(corr_packet pkt, char *data)
{
    pkt.data = (void *) (data+DATA_OFFSET);
}

void read_packet(corr_packet pkt, char *data)
{
    get_time(pkt,data);
    get_x_engines(pkt,data);
    get_data_offset(pkt,data);
    get_flags(pkt,data);
    get_data_length(pkt,data);
    get_data(pkt,data);
}


#endif


