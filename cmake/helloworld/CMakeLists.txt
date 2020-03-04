cmake_minimum_required(VERSION 3.16.4)

project (hello C ASM)

SET(CROSS_TOOLCHAIN_ROOT "/opt/gcc-arm-none-eabi-9-2019-q4-major")

SET(CMAKE_C_COMPILER   "${CROSS_TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc")
SET(CMAKE_CXX_COMPILER "${CROSS_TOOLCHAIN_ROOT}/bin/arm-none-eabi-g++")
SET(CMAKE_ASM_COMPILER "${CROSS_TOOLCHAIN_ROOT}/bin/arm-none-eabi-gcc")

SET(CMAKE_SYSROOT      "${CROSS_TOOLCHAIN_ROOT}/arm-none-eabi/lib")

SET(CMAKE_SYSTEM arm-cortex-m4)
SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR cortex-m4)
SET(CMAKE_EXE_LINKER_FLAGS "-specs=nosys.specs" CACHE STRING "" FORCE)

message(STATUS "Toolset: 'gcc9armeabi' (GNU Tools for Arm Embedded Processors, cross compiler)")

# Force cross-compilation mode
set(CMAKE_CROSSCOMPILING ON CACHE BOOL "Cross-compiling" FORCE)


add_compile_options(-mcpu=cortex-m4 -march=armv7e-m -mthumb -mlittle-endian -mfloat-abi=softfp -mfpu=fpv4-sp-d16)
add_compile_options(-fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -fno-move-loop-invariants)
add_compile_options(-nostartfiles)

add_compile_options(-g3)
add_compile_options(-DDEBUG=1)
add_compile_options(-DOS_LIBMODE_DT=1)

set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp -D__STARTUP_CLEAR_BSS")

add_compile_options("-Wno-conversion")
add_compile_options("-Wno-sign-conversion")
add_compile_options("-Wno-inline")
# C++ only
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-permissive")
# C only
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-incompatible-pointer-types -Wno-format")

add_link_options(-Wl,--gc-sections)

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Additional switches that GCC versions 8+ support. They need to be enabled explicitly.
# ...

# Additional switches that GCC versions 7+ support. They need to be enabled explicitly.
#add_compile_options("-Wduplicated-branches")

# Additional switches that GCC versions 6+ support. They need to be enabled explicitly.
add_compile_options("-Wduplicated-cond")
add_compile_options("-Wlogical-op")
add_compile_options("-Wshift-overflow=2")

# Additional switches that GCC versions 5+ support. They need to be enabled explicitly.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong")

# FIXME do not suppress this (C++ related) warning and resolve issues in source code
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-terminate")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")

include_directories(src)

# Files to Add
#file(GLOB SOURCES "src/*.c")
set(SOURCES src/main.c src/system_MIMX8MQ6_cm4.c src/startup.S)

SET(ASM_OPTIONS "-x assembler-with-cpp")
SET(CMAKE_ASM_FLAGS "${CFLAGS} ${ASM_OPTIONS}" )

add_executable(hello.elf ${SOURCES})

target_link_options(hello.elf PRIVATE -T "${CMAKE_CURRENT_SOURCE_DIR}/linker/MIMX8MQ6xxxJZ_cm4_ram.ld")
target_link_options(hello.elf PRIVATE --specs=nano.specs)

