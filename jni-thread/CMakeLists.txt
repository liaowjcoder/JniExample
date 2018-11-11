cmake_minimum_required(VERSION 3.4.1)

add_library(
             native-thread-lib
             SHARED
             src/main/cpp/native-thread-lib.cpp
              src/main/cpp/JavaListener.cpp
              )


target_link_libraries(
                       native-thread-lib
                      log )