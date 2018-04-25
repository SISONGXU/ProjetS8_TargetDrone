cmake_minimum_required(VERSION 2.8.3)
project(drone_test)

include_directories(

 include

   ${catkin_INCLUDE_DIRS}

   ${OpenCV_INCLUDE_DIRS}

)

find_package(catkin REQUIRED 
  COMPONENTS
  roscpp
  rospy
  std_msgs
  image_transport
  geometry_msgs
  cv_bridge
  image_transport
  sensor_msgs
)

find_package(Curses REQUIRED)
find_package( OpenCV REQUIRED )
## Declare a catkin package
catkin_package(INCLUDE_DIRS include LIBRARIES ${PROJECT_NAME}_keyboard_controller)

include_directories(include ${catkin_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})

## Librairies
add_library(${PROJECT_NAME}_keyboard_controller src/keyboard_controller.cpp)
target_link_libraries(${PROJECT_NAME}_keyboard_controller ${roscpp_LIBRARIES})

## Executables
add_executable(${PROJECT_NAME}_main_controller src/main_controller.cpp )
target_link_libraries(${PROJECT_NAME}_main_controller ${catkin_LIBRARIES} ${CURSES_LIBRARIES} ${PROJECT_NAME}_keyboard_controller)


add_executable(${PROJECT_NAME}_image_converter src/image_converter.cpp)
target_link_libraries(${PROJECT_NAME}_image_converter ${OpenCV_LIBRARIES} ${catkin_LIBRARIES}  )

add_executable(${PROJECT_NAME}_main_controller_rectangle src/main_controller_rectangle.cpp )
target_link_libraries(${PROJECT_NAME}_main_controller_rectangle ${catkin_LIBRARIES} ${CURSES_LIBRARIES} ${PROJECT_NAME}_keyboard_controller)

add_executable(${PROJECT_NAME}_main_controller_nv2 src/main_controller_nv2.cpp )
target_link_libraries(${PROJECT_NAME}_main_controller_nv2 ${catkin_LIBRARIES} ${CURSES_LIBRARIES} ${PROJECT_NAME}_keyboard_controller)
