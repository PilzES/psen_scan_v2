# Copyright (c) 2020-2021 Pilz GmbH & Co. KG
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.0.2)
project(psen_scan_v2)

add_compile_options(-std=c++14)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wno-unused-parameter)
add_compile_options(-Werror)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(catkin REQUIRED COMPONENTS
  rosconsole_bridge
  roscpp
  sensor_msgs
)

## System dependencies are found with CMake's conventions
find_package(Boost 1.69) ## Boost::system is header-only since 1.69
if(NOT Boost_FOUND)
  #catkin_lint: ignore_once shadowed_find
  find_package(Boost REQUIRED COMPONENTS system)
endif()
find_package(console_bridge REQUIRED)

catkin_package(
  CATKIN_DEPENDS
    roscpp
    sensor_msgs
)


###############
## Sanitizer ##
###############
# to run (only one sanitizer at a time):
# 1. Set environment variables for log_path in your shell
# export ASAN_OPTIONS="log_path=/path/to/asan-logs"
# export TSAN_OPTIONS="log_path=/path/to/tsan-logs"
# export UBSAN_OPTIONS="log_path=/path/to/ubsan-logs"
# 2. Compile
# catkin_make -DCATKIN_SANITIZER=address|thread|undefined
# 3. Run the node
# 4. Check log_path folder for contents
# build and devel folders have to be deleted before run
if(CATKIN_SANITIZER)
  set(SANITIZER_OPTIONS "address|thread|undefined")
  if("${CATKIN_SANITIZER}" MATCHES "${SANITIZER_OPTIONS}")
    message(STATUS "Activated: ${CATKIN_SANITIZER}-sanitizer")
    add_compile_options(-fsanitize=${CATKIN_SANITIZER} -g)
  else()
    message(FATAL_ERROR "${CATKIN_SANITIZER} is not a valid sanitizer. Valid options are: [${SANITIZER_OPTIONS}]")
  endif()
endif()

################
## Clang tidy ##
################
# to run: catkin_make -DCATKIN_ENABLE_CLANG_TIDY=true
# build and devel folders have to be deleted before run
if(CATKIN_ENABLE_CLANG_TIDY)
  find_program(
    CLANG_TIDY_EXE
    NAMES "clang-tidy"
    DOC "Path to clang-tidy executable"
    )
  if(NOT CLANG_TIDY_EXE)
    message(FATAL_ERROR "clang-tidy not found.")
  else()
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
  endif()
endif()
######################
## Build Standalone ##
######################

include_directories(
  include
  standalone/include
  ${console_bridge_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
)

set(${PROJECT_NAME}_standalone_sources
  standalone/src/scanner_v2.cpp
  standalone/src/laserscan.cpp
  standalone/src/data_conversion_layer/monitoring_frame_msg.cpp
  standalone/src/data_conversion_layer/start_request.cpp
  standalone/src/data_conversion_layer/start_request_serialization.cpp
  standalone/src/data_conversion_layer/stop_request_serialization.cpp
  standalone/src/data_conversion_layer/monitoring_frame_deserialization.cpp
  standalone/src/data_conversion_layer/diagnostics.cpp
  standalone/src/data_conversion_layer/scanner_reply_serialization_deserialization.cpp
)

###########
## Build ##
###########

set(${PROJECT_NAME}_node_sources
  src/psen_scan_driver.cpp
  ${${PROJECT_NAME}_standalone_sources}
)

add_executable(${PROJECT_NAME}_node ${${PROJECT_NAME}_node_sources})
target_link_libraries(${PROJECT_NAME}_node
  ${catkin_LIBRARIES}
)

#############
## Install ##
#############

install(DIRECTORY include/${PROJECT_NAME}/ standalone/include/${PROJECT_NAME}_standalone/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
  PATTERN ".svn" EXCLUDE
)

install(DIRECTORY launch DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
install(DIRECTORY config DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(DIRECTORY urdf/
  DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}/urdf)

install(TARGETS
  ${PROJECT_NAME}_node
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

