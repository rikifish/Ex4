cmake_minimum_required(VERSION 3.6)
project(Ex4)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES Socket.cpp Socket.h Udp.cpp
        Udp.h Grid.cpp Grid.h GridPt.cpp GridPt.h
        Point.cpp Point.h Bfs.cpp Bfs.h Passenger.cpp
        Passenger.h Trip.cpp Trip.h Driver.cpp Driver.h
        Luxury.cpp Luxury.h Map.cpp Map.h Node.cpp Node.h
        Taxi.cpp Taxi.h TaxiCenter.cpp)
#add_executable(Ex4 ${SOURCE_FILES})


set(BOOST_ROOT CACHE PATH "/usr/include/boost/") # or equivalent location
find_package(Boost 1.58.0 COMPONENTS serialization system) # or equivalent version

#add_library(Ex4 core ${core_SRCS})

add_executable(server TaxiFlow.cpp TaxiFlow.h ${SOURCE_FILES})
add_executable(client client.cpp ${SOURCE_FILES})


target_link_libraries(server ${Boost_LIBRARIES})
target_link_libraries(client ${Boost_LIBRARIES})
include_directories(${Boost_INCLUDE_DIR})