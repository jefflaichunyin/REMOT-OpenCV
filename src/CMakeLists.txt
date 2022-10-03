find_path(LZ4_INCLUDE_DIR NAMES lz4.h)
find_library(LZ4_LIBRARY NAMES lz4 REQUIRED)

add_executable(remot_opencv main.cpp)
target_sources(remot_opencv PRIVATE file_reader.cpp)
target_link_libraries(remot_opencv ${LZ4_LIBRARY})