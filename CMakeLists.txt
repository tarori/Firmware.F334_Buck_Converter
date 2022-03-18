cmake_minimum_required(VERSION 2.9)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
string(ASCII 27 Esc)

set(CMAKE_C_COMPILER arm-none-eabi-gcc CACHE STRING "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER arm-none-eabi-g++ CACHE STRING "arm-none-eabi-g++")
set(AS arm-none-eabi-gcc  CACHE STRING "arm-none-eabi-as")
set(CMAKE_AR arm-none-eabi-ar CACHE STRING "arm-none-eabi-ar")
set(CMAKE_LD arm-none-eabi-ld CACHE STRING "arm-none-eabi-ld")
set(NM arm-none-eabi-nm CACHE STRING "arm-none-eabi-nm")
set(OBJCOPY arm-none-eabi-objcopy CACHE STRING "arm-none-eabi-objcopy")
set(OBJDUMP arm-none-eabi-objdump CACHE STRING "arm-none-eabi-objdump")
set(READELF arm-none-eabi-readelf CACHE STRING "arm-none-eabi-readelf")
set(CMAKE_RANLIB arm-none-eabi-ranlib CACHE STRING "arm-none-eabi-ranlib")

enable_language(C)
enable_language(CXX)
enable_language(ASM)

# project name
project(F334_PS)
if(CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Release)
endif()

# set compiler option
set(COMPILER_OPT_FLAGS "-O2")
set(COMMON_COMPILER_FLAGS "-mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -DUSE_HAL_DRIVER -DSTM32F334x8  -fdata-sections -ffunction-sections -Wall -Wextra  -Wno-missing-field-initializers ${COMPILER_OPT_FLAGS}")
set(CMAKE_C_FLAGS "${COMMON_COMPILER_FLAGS} -std=gnu99")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_FLAGS_RELEASE "")
set(CMAKE_CXX_FLAGS "${COMMON_COMPILER_FLAGS} -std=gnu++17")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS "${COMMON_COMPILER_FLAGS}  -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS_DEBUG "-g")
set(CMAKE_ASM_FLAGS_RELEASE "")

# echo compile flags
message("Build type:\n\t${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message("Compile flags:\n\t${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message("Compile flags:\n\t${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
else()
    message("Compile flags:\n\t${CMAKE_CXX_FLAGS}")
endif()

include_directories("Inc" "CubeMX/F334_PS/Inc" "CubeMX/F334_PS/Drivers/CMSIS/Device/ST/STM32F3xx/Include" "CubeMX/F334_PS/Drivers/CMSIS/Include" "CubeMX/F334_PS/Drivers/STM32F3xx_HAL_Driver/Inc" "CubeMX/F334_PS/Drivers/STM32F3xx_HAL_Driver/Inc/Legacy")

file(GLOB PERIPH_LIB_SOURCE "CubeMX/F334_PS/Src/*.c" "CubeMX/F334_PS/Drivers/STM32F3xx_HAL_Driver/Src/*.c" "CubeMX/F334_PS/*.s")
list(REMOVE_ITEM PERIPH_LIB_SOURCE "${CMAKE_SOURCE_DIR}/CubeMX/F334_PS/Src/main.c")
list(REMOVE_ITEM PERIPH_LIB_SOURCE "${CMAKE_SOURCE_DIR}/CubeMX/F334_PS/Src/stm32f3xx_it.c")
add_library(periph-lib STATIC ${PERIPH_LIB_SOURCE})

set(CMAKE_EXE_LINKER_FLAGS "-mcpu=cortex-m4  -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -specs=nano.specs -T ${CMAKE_SOURCE_DIR}/CubeMX/F334_PS/STM32F334K8Tx_FLASH.ld -L. -lc -lm -u _printf_float -Wl,-Map=${CMAKE_PROJECT_NAME}.map,--cref -Wl,--gc-sections -Wl,--print-memory-usage")

file(GLOB MAIN_SOURCE "Src/*.cpp" "Src/*.c")

add_executable(${PROJECT_NAME}.elf ${MAIN_SOURCE})
target_link_libraries(${PROJECT_NAME}.elf periph-lib)
add_custom_target(${PROJECT_NAME}.hex ALL
    COMMAND arm-none-eabi-objcopy --output-format=ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex DEPENDS ${PROJECT_NAME}.elf)
#add_custom_target(write COMMAND /mnt/c/Program\ Files/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe -c port=SWD -d ./${PROJECT_NAME}.elf -s DEPENDS ${PROJECT_NAME}.hex)

add_custom_target(write COMMAND ${CMAKE_SOURCE_DIR}/program.sh ${PROJECT_NAME}.elf -s DEPENDS ${PROJECT_NAME}.elf)
