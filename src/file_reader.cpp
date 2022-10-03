#include <iostream>


#include "file_reader.hpp"

File_Reader::File_Reader(std::string filename, std::queue<Event> *event_queue)
{
    std::ifstream infile;
    infile.open(filename, std::ios::binary | std::ios::in);

    if (infile.fail())
    {
        std::cout << "Failed to open the input file" << std::endl;
        throw std::runtime_error("Failed to open the input file");
    }
    std::cout << "File opened\n";

    infile.seekg(0, std::ios::end);
    int file_size = infile.tellg();
    infile.seekg(0, std::ios::beg);
    char *data = new char[file_size];
    char *buffer_start = data;

    infile.read(data, file_size); // TODO: use mmap
    infile.close();

    std::cout << "Read " << file_size << " bytes\n";

    auto header = std::string(data, 14);
    std::cout << "Header: " << header << std::endl;
    if (header != "#!AER-DAT4.0\r\n")
    {
        throw std::runtime_error("Invalid AEDAT version");
    }

    data += 14; // size of the version string
    flatbuffers::uoffset_t ioheader_offset =
        *reinterpret_cast<flatbuffers::uoffset_t *>(data);
    std::cout << "IO header offset = " << ioheader_offset << std::endl;

    const IOHeader *ioheader = GetSizePrefixedIOHeader(data);
    int64_t data_table_position = ioheader->data_table_position();
    std::cout << "Data table position = " << data_table_position << std::endl;
    if (data_table_position < 0)
    {
        throw std::runtime_error(
            "AEDAT files without datatables are currently not supported");
    }
    char *data_table_start = buffer_start + data_table_position;

    data += ioheader_offset + 4;
    // start of data stream
    LZ4F_errorCode_t lz4_error =
        LZ4F_createDecompressionContext(&lz4_ctx, LZ4F_VERSION);

    if (LZ4F_isError(lz4_error))
    {
        throw std::runtime_error("Decompression context error");
    }
    
    data_cur = (uint8_t*)data;
    data_start = (uint8_t*)data;
    data_end = (uint8_t*)data_table_start;
}

const EventPacket * File_Reader::read_packet(int &event_cnt){
    static uint8_t dst_buf[DECOMPRESSION_BUF_SIZE];
    int32_t stream_id;
    size_t pkt_size;

    stream_id = *reinterpret_cast<int32_t *>(data_cur);
    data_cur += 4;
    pkt_size = *reinterpret_cast<int32_t *>(data_cur);
    data_cur += 4;

    while(stream_id != 0 && data_cur < data_end){
        data_cur += pkt_size;
        stream_id = *reinterpret_cast<int32_t *>(data_cur);
        data_cur += 4;
        pkt_size = *reinterpret_cast<int32_t *>(data_cur);
        data_cur += 4;
    }
    
    std::cout << "stream id: " << stream_id << " size: " << pkt_size << std::endl;

    if(stream_id != 0)
    {
        event_cnt = 0;
        return nullptr;
    }


    // event data
    size_t read_len = pkt_size;
    size_t write_len = sizeof(dst_buf);
    size_t remain = LZ4F_decompress(lz4_ctx, dst_buf, &write_len, data_cur, &read_len, nullptr);
    data_cur += pkt_size;
    std::cout << "read: " << read_len << " write: " << write_len << std::endl;
    if (remain > 0)
    {
        throw std::runtime_error("Incomplete decompression, increase buffer size");
        event_cnt = 0;
        return nullptr;
    }

    auto event_pkt = GetSizePrefixedEventPacket(dst_buf);
    event_cnt = event_pkt->elements()->Length();
    return event_pkt;
}   