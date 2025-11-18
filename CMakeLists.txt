cmake_minimum_required(VERSION 3.30...4.0)

project(otfcc
  HOMEPAGE_URL "https://github.com/InCom-0/otfcc"
  DESCRIPTION
  "Optimized OpenType builder and inspector."
  LANGUAGES CXX C)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

#####################################################################
### Platform specific hacks ###
#####################################################################
if(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MATCHES "MSVC")
  add_compile_options(/utf-8)
  add_compile_definitions(NOMINMAX)
endif()


########################################################
### Target specification ###
########################################################
add_subdirectory(dep)

add_library(otfcc-lib STATIC)
add_library(otfcc-lib::otfcc-lib ALIAS otfcc-lib)

file(GLOB_RECURSE SRC_otfcc-lib src/**.c)
set_source_files_properties(
    ${SRC_otfcc-lib}
    PROPERTIES LANGUAGE C
)
target_sources(otfcc-lib PRIVATE ${SRC_otfcc-lib})

target_sources(otfcc-lib
  PUBLIC
  FILE_SET pub_headers_insource
  TYPE HEADERS
  BASE_DIRS
  src)

target_sources(otfcc-lib
  PUBLIC
  FILE_SET pub_headers
  TYPE HEADERS
  BASE_DIRS
  include)

target_link_libraries(otfcc-lib PRIVATE dep-lib)
