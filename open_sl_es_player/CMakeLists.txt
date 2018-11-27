
cmake_minimum_required(VERSION 3.4.1)

add_library(
             opensles_lib
             SHARED
             src/main/cpp/opensles_lib.cpp
           )



target_link_libraries(opensles_lib
                       OpenSLES
                       log )