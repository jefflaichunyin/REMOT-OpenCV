#include <iostream>
#include <opencv2/opencv.hpp>
#include "file_reader.hpp"

#define AEDAT_PATH      "../../samples/footbridge_away.aedat4"
#define FRAME_WIDTH     346
#define FRAME_HEIGHT    260

using namespace cv;
using namespace std;

int main(int, char**) {
    Mat frame(FRAME_HEIGHT, FRAME_WIDTH, CV_8UC3, Scalar::all(0));
    namedWindow("Event output", WINDOW_NORMAL);

    int event_cnt;
    std::queue<Event> event_queue;
    File_Reader file_reader(AEDAT_PATH, &event_queue);
    do{
        auto event_pkt = file_reader.read_packet(event_cnt);
        std::cout << "Extracted " << event_cnt  << " events" << std::endl;
        if(event_cnt > 0){
            frame = Scalar(0,0,0);
            for(auto event : *event_pkt->elements()){
                frame.at<Vec3b>(event->y(), event->x()) = event->on() ? Vec3b(0,0,255) : Vec3b(0,255,0);
            }
            imshow("Event output", frame);
            waitKey(100);
        }
    }while(event_cnt > 0);
}
