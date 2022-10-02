find_path(LZ4_INCLUDE_DIR NAMES lz4.h)
find_library(LZ4_LIBRARY NAMES lz4 REQUIRED)

add_executable(aedat-reader main.cpp)
target_link_libraries(aedat-reader ${LZ4_LIBRARY})