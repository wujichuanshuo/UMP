cmake_minimum_required( VERSION 2.8 )

PROJECT(memory_report_decoder)

set(CMAKE_CXX_STANDARD 11)

if (MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
endif (MSVC)

add_executable(memory_report_decoder 
    src/main.cpp 
    src/umpcrawler.cpp
    #src/Windows.cpp
)

#include_directories(${FFMPEG_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
target_link_libraries(memory_report_decoder ${FFMPEG_LIBRARIES} ${SDL2_LIBRARIES})
