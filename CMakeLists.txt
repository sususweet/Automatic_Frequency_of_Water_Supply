cmake_minimum_required(VERSION 3.7)
project(WaterSupply)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES settings.h
        LCD12864.h
        ADC.h
        main.c
        LCD12864.c
        ADC.c
        intrinsics.h
        Keyboard.c
        Keyboard.h
        ThreePhaseSpwm.c
        ThreePhaseSpwm.h
        frequency_capture.c
        frequency_capture.h
        in430.h
        msp430f5438a.h)

# include_directories(.)

add_executable(F5438_LED ${SOURCE_FILES})