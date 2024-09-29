cmake_minimum_required(VERSION 3.0.2)
project(fire_robot)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

find_package(catkin REQUIRED COMPONENTS
  roscpp
  std_msgs
  nav_msgs
  sensor_msgs
  tf
  turtlebot3_msgs
  geometry_msgs
  cv_bridge
  image_transport
)
include_directories(
 # include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

catkin_package(
#  INCLUDE_DIRS include
  LIBRARIES fire_robot
  CATKIN_DEPENDS roscpp std_msgs nav_msgs sensor_msgs tf turtlebot3_msgs geometry_msgs
#  DEPENDS system_lib
)


## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
#   DEPENDENCIES
#   std_msgs
# )

## Qt
set(CMAKE_PREFIX_PATH "/home/ubuntu/Qt5.14.2/5.14.2/gcc_64")
set(ModuleList 
    Core
    Gui
    Widgets
    Multimedia
    Network
)

foreach(QtModule ${ModuleList})
    set(Qt5${QtModule}_DIR ${CMAKE_PREFIX_PATH}/lib/cmake/Qt5${QtModule})
    message("foreach-> Qt5${QtModule}_DIR : ${Qt5${QtModule}_DIR}")

    set(QT_LIBRARIES ${QT_LIBRARIES} Qt5::${QtModule})
endforeach()
message("${QT_LIBRARIES}")

find_package(Qt5 REQUIRED ${ModuleList})
find_package(Qt5Widgets REQUIRED })
find_package(OpenCV)

file(GLOB QT_FORMS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ui/*.ui)
file(GLOB QT_RESOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.qrc)
file(GLOB_RECURSE QT_MOC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS include/fire_robot/*.hpp include/fire_robot/*.h)

QT5_WRAP_UI(QT_FORMS_HPP ${QT_FORMS})
QT5_WRAP_CPP(QT_MOC_HPP ${QT_MOC})

file(GLOB_RECURSE QT_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} FOLLOW_SYMLINKS src/*.cpp)
set(QT_SOURCES ${QT_SOURCES} src/tab3mapping.cpp)
add_executable(rosqt ${SOURCE} ${QRC_FILES} ${QT_SOURCES} ${QT_RESOURCES_CPP} ${QT_FORMS_HPP} ${QT_MOC_HPP} )
target_link_libraries(rosqt ${QT_LIBRARIES} ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
install(TARGETS rosqt RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION})

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")

