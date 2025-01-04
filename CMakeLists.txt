# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(FFautologon CXX RC)

# UTF-8 source code support
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
endif()

# JAPAN or not?
option(JAPAN "Enable Japanese support" ON)
if(JAPAN)
    add_definitions(-DJAPAN)
endif()
message(STATUS "JAPAN = ${JAPAN}")

# FFautologon.exe
add_executable(FFautologon FFautologon.cpp FFautologon_res.rc)
target_compile_definitions(FFautologon PRIVATE UNICODE _UNICODE)
target_link_libraries(FFautologon comctl32 shlwapi)

# Don't add default manifest
if(MSVC)
    target_link_options(FFautologon PRIVATE /MANIFEST:NO)
endif()

##############################################################################
