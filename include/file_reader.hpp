#define DECOMPRESSION_BUF_SIZE 1000000

#include <queue>
#include <fstream>
#include <lz4.h>
#include <lz4frame.h>

#include <flatbuffers/flatbuffers.h>
#include "flatbuffers/ioheader_generated.h"
#include "flatbuffers/file_data_table_generated.h"
#include "flatbuffers/events_generated.h"

class File_Reader{
    public:
        File_Reader(std::string filename, std::queue<Event> *event_queue);
        const EventPacket * read_packet(int &event_cnt);
    private:
        LZ4F_decompressionContext_t lz4_ctx;
        uint8_t *data_cur;
        uint8_t *data_start;
        uint8_t *data_end;
};