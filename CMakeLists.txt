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

# The options that is configurable in this module.
option(EXOTIC_RUNTEST "Build and run tests to of exotic (GoogleTest required)." ON)
option(EXOTIC_DOXYGEN "Generate documents for exotic (Doxygen required)." ON)

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

# Configure the tests, only if EXOTIC_RUNTEST is enabled.
if(${EXOTIC_RUNTEST})
message("EXOTIC_RUNTEST is enabled, configuring tests while building.")

# Enable CMake testings first.
include(CTest)
enable_testing()

# Ensure that GoogleTest is configured properly on testing machine.
find_package(GTest REQUIRED)
if(NOT ${GTEST_FOUND})
message(SEND_ERROR "EXOTIC_RUNTEST is enabled, but GoogleTest is not configured.")
endif()

# Defines the adding test macro for single file test, as a helper.
macro(exotic_add_test EXOTIC_TESTNAME)
add_executable("${EXOTIC_TESTNAME}.test" "${CMAKE_CURRENT_SOURCE_DIR}/${EXOTIC_TESTNAME}.cpp")
target_link_libraries("${EXOTIC_TESTNAME}.test" GTest::GTest GTest::Main)
add_test("exotic.${EXOTIC_TESTNAME}" "${EXOTIC_TESTNAME}.test")
endmacro(exotic_add_test)

# Add a 'check' target to verbosely output the google test result.
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure)

# Also include and build the test cases under the test directory.
add_subdirectory(test)
endif()

# Configure the docs, only if EXOTIC_DOXYGEN is enabled.
if(${EXOTIC_DOXYGEN})
message("EXOTIC_DOXYGEN is enabled, generate documents while building.")

# Ensure that Doxygen is configured properly on building machine.
find_package(Doxygen REQUIRED)
if(NOT ${DOXYGEN_FOUND})
message(SEND_ERROR "EXOTIC_DOXYGEN is enabled, but Doxygen is not configured.")
endif()

# Configure the Doxyfile.in to the output Doxyfile configuration.
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/doc/Doxyfile.in" 
	"${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)
	
# Add custom target for generating doxygen doc.
add_custom_target(doc ALL
	COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
	WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
	COMMENT "Generating documents using Doxygen." VERBATIM)

endif()