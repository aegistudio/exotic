# Exotic: Metaprogramming-based embedded data structures.
# @author: Haoran Luo
#
# Exotic is a header only c++ library utilizing metaprogramming to create data 
# structures with fields embedded in object. This could reduce performance 
# degrading and external fragmentation caused by exccessive allocation/deallocation. 
# And it could also be used in some scenario that dynamic memory allocation is not
# desired, like embedded environment and low performance machines.
cmake_minimum_required(VERSION 3.5)
project(exotic)

# Ensure the project uses (at least) c++11.
set(CMAKE_CXX_STANDARD 11)


# The include directory of the project.
include_directories(include)

# Make the interface only library for the exotic.
add_library(exotic INTERFACE)
target_include_directories(exotic INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/exotic>
	$<INSTALL_INTERFACE:include/exotic>)
install(FILES
	# Basic definitions for the library.
	"${CMAKE_CURRENT_SOURCE_DIR}/include/exotic/node.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/exotic/iterator.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/exotic/key.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/include/exotic/scope.hpp"
	
	# Implemented data structures of the library.
	"${CMAKE_CURRENT_SOURCE_DIR}/include/exotic/list.hpp"
	DESTINATION "include/exotic")
install(TARGETS exotic EXPORT exoticCmakeExport)
install(EXPORT exoticCmakeExport DESTINATION "lib/cmake")