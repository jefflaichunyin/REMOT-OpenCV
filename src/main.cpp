#include <iostream>
#include <fstream>
#include <lz4.h>
#include <lz4frame.h>

#include <flatbuffers/flatbuffers.h>
#include "flatbuffers/ioheader_generated.h"
#include "flatbuffers/file_data_table_generated.h"
#include "flatbuffers/events_generated.h"

#define DECOMPRESSION_BUF_SIZE 1000000
int main(int, char**) {
    std::ifstream infile;
    infile.open("../../samples/footbridge_away.aedat4", std::ios::binary | std::ios::in);

    if(infile.fail()){
        std::cout << "Failed to open the input file" << std::endl;
        return -1;
    }
    std::cout << "File opened\n";

    infile.seekg(0,std::ios::end);
    int file_size = infile.tellg();
    infile.seekg(0,std::ios::beg);
    char *data = new char[file_size];
    char *buffer_start = data;

    infile.read(data, file_size); // TODO: use mmap
    infile.close();

    std::cout << "Read " << file_size << " bytes\n";

    auto header = std::string(data, 14);
    std::cout << "Header: " << header << std::endl;
    if (header != "#!AER-DAT4.0\r\n") {
      throw std::runtime_error("Invalid AEDAT version");
    }
    
    data += 14; // size of the version string
    flatbuffers::uoffset_t ioheader_offset =
        *reinterpret_cast<flatbuffers::uoffset_t *>(data);
    std::cout << "IO header offset = " << ioheader_offset << std::endl;

    const IOHeader *ioheader = GetSizePrefixedIOHeader(data);
    int64_t data_table_position = ioheader->data_table_position();
    std::cout << "Data table position = " << data_table_position << std::endl;
    if (data_table_position < 0) {
      throw std::runtime_error(
          "AEDAT files without datatables are currently not supported");
    }
    char *data_table_start = buffer_start + data_table_position;

    data += ioheader_offset + 4;
    // start of data stream
    uint8_t dst_buf[DECOMPRESSION_BUF_SIZE];
    LZ4F_decompressionContext_t ctx;
    LZ4F_errorCode_t lz4_error =
        LZ4F_createDecompressionContext(&ctx, LZ4F_VERSION);

    if (LZ4F_isError(lz4_error)) {
      printf("Decompression context error: %s\n", LZ4F_getErrorName(lz4_error));
      return -1;
    }

    while(data < data_table_start){
        int32_t stream_id = *reinterpret_cast<int32_t *>(data);
        data += 4;
        size_t size = *reinterpret_cast<int32_t *>(data);
        data += 4;

        std::cout << "stream id: " << stream_id << " size: " << size << std::endl;

        if(stream_id == 0){
            // event data
            size_t read_len = size;
            size_t write_len = sizeof(dst_buf);
            size_t remain = LZ4F_decompress(ctx, dst_buf, &write_len, data, &read_len, nullptr);
            std::cout << "read: " << read_len << " write: " << write_len << std::endl;
            if(remain > 0){
                throw std::runtime_error("Incomplete decompression");
            }
            auto event_pkt = GetSizePrefixedEventPacket(dst_buf);
            for (auto event : *event_pkt->elements()) {
                printf("%d %ld %d %d\n", event->on(), event->t(), event->x(), event->y());
            }
        }

        data += size;
    }
}
