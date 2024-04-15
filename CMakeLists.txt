# SLIDEM Processor: CMakeLists.txt

# Copyright (C) 2024 Johnathan K Burchill

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

PROJECT(slidem)

CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

ADD_DEFINITIONS(-DHAVE_GETOPT_H)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Debugging enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -std=gnu99 ")
else (CMAKE_BUILD_TYPE STREQUAL Debug)
    message( "-- Optimization enabled")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -std=gnu99 ")
endif(CMAKE_BUILD_TYPE STREQUAL Debug)

# GSL
FIND_PACKAGE(GSL REQUIRED)

# LIBXML2
FIND_PACKAGE(LibXml2)

# requires -lm on linux
FIND_LIBRARY(MATH m)

SET(LIBS ${LIBS} ${MATH} ${GSL_LIBRARY} ${LIBXM2_LIBRARIES})

INCLUDE_DIRECTORIES(${INCLUDE_DIRS} ${GSL_INCLUDE_DIRS} ${ZIP_INCLUDE_DIRS} ${HOME}/include ${LIBXML2_INCLUDE_DIR})

ADD_EXECUTABLE(slidem0301 main.c cdf_vars.c cdf_attrs.c load_inputs.c downsample.c interpolate.c modified_oml.c calculate_products.c export_products.c utilities.c post_process.c ioncomposition.c calion.c iri2016util.c f107.c load_satellite_velocity.c calculate_diplatitude.c write_header.c)
TARGET_INCLUDE_DIRECTORIES(slidem0301 PRIVATE ${HOME}/include)
TARGET_LINK_LIBRARIES(slidem0301 ${LIBS} -lgslcblas -lgsl -lcdf -lxml2)

install(TARGETS slidem0301 DESTINATION $ENV{HOME}/bin)

# SLIDEM config file
install(FILES slidem_modified_oml_configrc DESTINATION $ENV{HOME} RENAME .slidem_modified_oml_configrc)

add_subdirectory(util)
