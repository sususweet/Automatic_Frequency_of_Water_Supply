cmake_minimum_required(VERSION 3.7)
project(WaterSupply)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES settings.h LCD12864.h ADC.h main.c LCD12864.c ADC.h intrinsics.h Keyboard.c Keyboard.h ThreePhaseSpwm.c ThreePhaseSpwm.h)

# include_directories(.)

add_executable(F5438_LED ${SOURCE_FILES})