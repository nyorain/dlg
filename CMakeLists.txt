# Copyright (c) 2017 nyorain
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

cmake_minimum_required (VERSION 2.8.0)
project(dlg)

option(static "build the library static" on)
option(sample "Build the sample" off)

# enable warnings
set(is_clang CMAKE_CXX_COMPILER_ID MATCHES "Clang")
if(MSVC)
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR is_clang)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -Wextra -Wpedantic")
endif()

# c++1z
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")
include_directories("include")

# library
if(Static)
	add_library(dlg STATIC "src/dlg/dlg.cpp")
else()
	add_library(dlg SHARED "src/dlg/dlg.cpp")
endif()

#install
install(TARGETS dlg
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)

install(DIRECTORY include/dlg DESTINATION include)

# sample
if(Sample)
	add_executable(sample "docs/example.cpp")
	target_link_libraries(sample dlg)
endif()
