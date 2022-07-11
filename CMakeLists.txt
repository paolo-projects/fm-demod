cmake_minimum_required(VERSION 3.7)

project(FmDemod)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/modules")
set(SRCS src/FmDemodulator.cpp)

add_library(FmDemod SHARED ${SRCS})
add_library(FmDemodStatic STATIC ${SRCS})

target_include_directories(FmDemod PUBLIC include)
target_include_directories(FmDemodStatic PUBLIC include)

set_target_properties(FmDemod PROPERTIES CMAKE_CXX_FLAGS_RELEASE "-O3")
set_target_properties(FmDemod PROPERTIES CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set_target_properties(FmDemodStatic PROPERTIES CMAKE_CXX_FLAGS_RELEASE "-O3")
set_target_properties(FmDemodStatic PROPERTIES CMAKE_CXX_FLAGS_DEBUG "-g -O0")

find_package(Threads REQUIRED)
target_link_libraries(FmDemod Threads::Threads)
target_link_libraries(FmDemodStatic Threads::Threads)

find_package(FFTW REQUIRED)

target_link_libraries(FmDemod ${FFTW_LIBRARIES})
target_include_directories(FmDemod PUBLIC ${FFTW_INCLUDE_DIRS})
target_link_libraries(FmDemodStatic ${FFTW_LIBRARIES})
target_include_directories(FmDemodStatic PUBLIC ${FFTW_INCLUDE_DIRS})