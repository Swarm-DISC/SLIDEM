project(slidem)


CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

ADD_EXECUTABLE(printSortedVar main.c)
TARGET_LINK_LIBRARIES(printSortedVar -lcdf)

