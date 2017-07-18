cmake_minimum_required(VERSION 3.7)
project(WaterSupply)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES settings.h LCD12864.h ads1118.h main.c LCD12864.c ads1118.c intrinsics.h)

# include_directories(.)

add_executable(F5438_LED ${SOURCE_FILES})