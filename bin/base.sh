#!/bin/bash

# Find Base repo dir
base=$(readlink -f $(dirname "${BASH_SOURCE[0]}")/..)
prj=$(basename $(readlink -f .))

source $base/bin/lib.sh

mkdir -p bld chk cmake src
> bld/.keep_as_build_directory

if ! [ -r LICENSE ]; then
	cp $base/LICENSE .
	echo "I wrote a BSD-3 license file for you. You might want to check and"
	echo "edit it first. When you're satisfied with it, please run this"
	echo "script again to complete the base setup and layout. The generated"
	echo "files will then all contain a copy of the LICENSE file."
	exit 0
fi

if ! [ -r .gitignore ]; then
cat > .gitignore <<EOF
.*.swp
bld/**
!bld/.keep_as_build_directory
tags
EOF
fi

if ! [ -r chk/chk.cpp ]; then
writeCppLicense ./LICENSE > chk/chk.cpp
cat >>chk/chk.cpp <<EOF

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

/** Runs the check application.
 * @returns 0 on success, 1 if a check failed. */
int main(void)
{
	CppUnit::TextUi::TestRunner runner;
	bool retval = false;

	// Set up test suite
	CppUnit::TestFactoryRegistry & registry =
		CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());

	// Actually run all tests
	retval = runner.run();

	// Convert the C-style true (1) to Shell-style true (0) and v.v.
	return retval ? 0 : 1;
}
EOF
fi

if ! [ -r ./CMakeLists.txt ]; then
writeBashLicense ./LICENSE > ./CMakeLists.txt
cat >>./CMakeLists.txt <<EOF
cmake_minimum_required (VERSION 3.4)
if(POLICY CMP0015)
	cmake_policy(SET CMP0015 NEW)
endif()
if(POLICY CMP0056)
	cmake_policy(SET CMP0056 NEW)
endif()

list (APPEND CMAKE_MODULE_PATH "\${CMAKE_SOURCE_DIR}/cmake")

project ($prj)

# Set the Git repository root directory
execute_process (
	COMMAND git rev-parse --show-toplevel
	OUTPUT_VARIABLE GITREPOROOT
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Offer option to do code coverage when build type is Debug
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	option (RUN_COVERAGE "Analyze the code coverage of the unitchecks" ON)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

# Custom C/C++ flags
set (CUSTOM_FLAGS   "-Wall -Werror -Wextra -Wno-varargs -DGITREPOROOT='\${GITREPOROOT}'")
set (CUSTOM_DEBUG   "-O0 -g3 -ggdb")
set (CUSTOM_RELEASE "-O3")

# Common linker flags
set (CMAKE_EXE_LINKER_FLAGS_DEBUG   "\${CMAKE_EXE_LINKER_FLAGS}")
set (CMAKE_EXE_LINKER_FLAGS_RELEASE "\${CMAKE_EXE_LINKER_FLAGS} -s")

# Plain C flags
set (CMAKE_C_FLAGS         "\${CUSTOM_FLAGS}")
set (CMAKE_C_FLAGS_DEBUG   "\${CUSTOM_DEBUG}")
set (CMAKE_C_FLAGS_RELEASE "\${CUSTOM_RELEASE}")

# C++ flags, mostly the same as C
set (CMAKE_CXX_FLAGS         "\${CUSTOM_FLAGS} -std=gnu++17")
set (CMAKE_CXX_FLAGS_DEBUG   "\${CUSTOM_DEBUG}")
set (CMAKE_CXX_FLAGS_RELEASE "\${CUSTOM_RELEASE}")

# Find necessary packages
find_package (Boost REQUIRED
	COMPONENTS system
)
find_package (Threads REQUIRED)

# Include stuff for code coverage
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
	INCLUDE(CodeCoverage)
	SETUP_TARGET_FOR_COVERAGE(cov$prj chk ${prj}cov)
	add_custom_target(RunCoverage ALL DEPENDS cov$prj)
endif (CMAKE_BUILD_TYPE STREQUAL "Debug")

add_subdirectory (src)
add_subdirectory (chk)

add_custom_target (BuildApp ALL DEPENDS $prj)
EOF
fi

if ! [ -r src/CMakeLists.txt ]; then
writeBashLicense ./LICENSE > src/CMakeLists.txt
cat >>src/CMakeLists.txt <<EOF

include_directories (
        \${Boost_INCLUDE_DIRS}
)

add_library ($prj SHARED
        Logger.cpp
)

target_link_libraries ($prj
        \${Boost_LIBRARIES}
        \${CMAKE_THREAD_LIBS_INIT}
)

add_executable (${prj}cli ${prj}cli.cpp)
target_link_libraries (${prj}cli $prj)
EOF
fi

if ! [ -r chk/CMakeLists.txt ]; then
writeBashLicense ./LICENSE > chk/CMakeLists.txt
cat >>chk/CMakeLists.txt <<EOF

# Include stuff for code coverage
if (RUN_COVERAGE STREQUAL "ON")
	include(CodeCoverage)
	append_coverage_compiler_flags()
	setup_target_for_coverage_gcovr_html(
		NAME coverage
		EXECUTABLE chk
		BASE_DIRECTORY "${PROJECT_SOURCE_DIR}/src"
		EXCLUDE "${PROJECT_SOURCE_DIR}/chk"
	)
endif (RUN_COVERAGE STREQUAL "ON")

# Find the CppUnit package
find_package (CppUnit REQUIRED)

include_directories (
	\${CPPUNIT_INCLUDE_DIR}
	\${CMAKE_SOURCE_DIR}/src
)

add_executable (chk
   	chk.cpp
)

target_link_libraries (chk
	$prj
	\${CPPUNIT_LIBRARY}
)
EOF
fi
