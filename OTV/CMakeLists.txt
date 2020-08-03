cmake_minimum_required(VERSION 3.5.1)
set (CMAKE_BUILD_TYPE Release)
SET (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
add_definitions(-std=c++0x)
project(river_flow_velocity_estimation)
set( OpenCV_DIR "/usr/share/OpenCV" )
#set(OPENCV_ENABLE_NONFREE "Enable non-free algorithms" ON)
find_package( OpenCV REQUIRED)
include_directories(include ${OpenCV_LIBRARIES} ${OpenCV_INCLUDE_DIRS} } ${catkin_INCLUDE_DIRS})
file(GLOB SOURCES "src/*.cpp")
add_executable(river_flow_velocity_estimation ${SOURCES})
#opencv_nonfree
target_link_libraries(river_flow_velocity_estimation  ${OpenCV_LIBS} ${catkin_LIBRARIES})

