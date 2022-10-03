#include <iostream>
#include "file_reader.hpp"

#define AEDAT_PATH "../../samples/footbridge_away.aedat4"

int main(int, char**) {
    int event_cnt;
    std::queue<Event> event_queue;
    File_Reader file_reader(AEDAT_PATH, &event_queue);
    do{
        file_reader.read_packet(event_cnt);
        std::cout << "Extracted " << event_cnt  << " events" << std::endl;
    }while(event_cnt > 0);
}
