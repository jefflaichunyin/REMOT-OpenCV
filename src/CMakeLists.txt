find_library(LZ4_LIBRARY NAMES lz4 REQUIRED)
find_package( OpenCV REQUIRED )

add_executable(remot_opencv main.cpp)
target_sources(remot_opencv PRIVATE file_reader.cpp)
target_link_libraries(remot_opencv ${LZ4_LIBRARY} ${OpenCV_LIBS}) 