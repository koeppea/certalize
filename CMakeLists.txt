cmake_minimum_required(VERSION 2.8)
project(certalize C)

set(VERSION "0.0.1")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")
set(CMAKE_SCRIPT_PATH "${CMAKE_SOURCE_DIR}/cmake/Scripts")

include(MacroEnsureOutOfSourceBuild)
macro_ensure_out_of_source_build("${PROJECT_NAME} requires an out of
  source build. Please create a separate build directory and run
  'cmake /path/to/${PROJECT_NAME} [options]' there.")

set(VALID_BUILD_TYPES Debug Release)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Choose the type of build, options are: ${VALID_BUILD_TYPES}." FORCE)
endif() 
list(FIND VALID_BUILD_TYPES ${CMAKE_BUILD_TYPE} contains_valid)
if(contains_valid EQUAL -1)
  message(FATAL_ERROR "Unknown CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}'.
    Valid options are: ${VALID_BUILD_TYPES}")
endif()
unset(contains_valid)

include(CheckIncludeFile)

check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(ctype.h HAVE_CTYPE_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(getopt.h HAVE_GETOPT_H)

set(LIBS)
set(INCLUDE_DIRS)

set(GTK3_FIND_VERSION 1)
find_package(GTK3 3.12.0 REQUIRED)
if(GTK3_FOUND)
  set(HAVE_GTK3 1)
else()
  message(FATAL_ERROR "GTK3 library not found")
endif()

set(LIBS ${LIBS} ${GTK3_LIBRARIES})
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
include_directories(${GTK3_INCLUDE_DIRS})


set(INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "Installation prefix")
set(INSTALL_LIBDIR ${INSTALL_PREFIX}/lib${LIB_SUFFIX} CACHE PATH "Library installation prefix")
set(INSTALL_DATADIR ${INSTALL_PREFIX}/share CACHE PATH "Data installation directory")
set(INSTALL_UIDIR ${INSTALL_DATADIR}/${PROJECT_NAME} CACHE PATH "UI resource file directory")
set(INSTALL_BINDIR ${INSTALL_PREFIX}/bin CACHE PATH "Binary files installation directory")
set(DESKTOP_DIR ${INSTALL_PREFIX}/share/applications CACHE PATH "Desktop files installation directory")
set(ICON_DIR ${INSTALL_PREFIX}/share/pixmaps CACHE PATH "Icon file installation directory")
set(MAN_DIR ${INSTALL_PREFIX}/share/man CACHE PATH "Path for manual pages")


# set general build flags for debug build-type
set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3 -DDEBUG -Wall -Wno-pointer-sign -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -Wextra -Wredundant-decls" CACHE STRING "" FORCE)

# set build flags for release build-type
set(CMAKE_C_FLAGS_RELEASE "-O2 -w -D_FORTIFY_SOURCE=2" CACHE STRING "" FORCE)


set(INCLUDE_PATH ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_SOURCE_DIR}/include)
include_directories(${INCLUDE_PATH})

add_subdirectory(src)
add_subdirectory(ui)



# This line should ALWAYS be after all options are defined
configure_file(include/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/include/config.h)


# uninstall target
configure_file(
    "${CMAKE_SCRIPT_PATH}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

# Add a target that will ensure that the build directory is properly cleaned.
add_custom_target(clean-all
  COMMAND ${CMAKE_BUILD_TOOL} clean
  COMMAND ${CMAKE_COMMAND} -P ${CMAKE_SCRIPT_PATH}/clean-all.cmake
)



include(FeatureSummary)
feature_summary(WHAT ALL)



