cmake_minimum_required(VERSION 3.9.4) 
project(WinGadgetLib CXX C)

# The version number.
set(WINGADGETLIB_VERSION_MAJOR 1)
set(WINGADGETLIB_VERSION_MINOR 0)

# Custom cmake search
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules/")

#Find boost
find_package(Boost 1.60 REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Boost libraries were not found")
endif(NOT Boost_FOUND)

#Find NTL
find_package(NTL REQUIRED)
if(NOT NTL_FOUND)
	message(FATAL_ERROR "NTL was not found")
endif(NOT NTL_FOUND)

add_subdirectory(sources)

#Enable ctest in the future
#include(CTest)





