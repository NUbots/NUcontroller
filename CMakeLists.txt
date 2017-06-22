CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(CM730)

ENABLE_LANGUAGE(ASM)

# Set device options.
SET(DEVICE CM730 CACHE STRING "The device to target")
SET_PROPERTY(CACHE DEVICE PROPERTY STRINGS CM730 CM740)

SET(BATTERY_TYPE BATTERY_4CELL CACHE STRING "The type of battery to use")
SET_PROPERTY(CACHE BATTERY_TYPE PROPERTY STRINGS BATTERY_3CELL BATTERY_4CELL)

# Set project output files.
SET(ELF_FILE ${PROJECT_NAME}.elf)
SET(MAP_FILE ${PROJECT_NAME}.map)
SET(HEX_FILE ${PROJECT_NAME}.hex)
SET(BIN_FILE ${PROJECT_NAME}.bin)
SET(LSS_FILE ${PROJECT_NAME}.lss)

ADD_DEFINITIONS(-DFORCE_${DEVICE}
                -D${BATTERY_TYPE})

INCLUDE_DIRECTORIES(.
                    CM730_HW/inc
                    CM730_APP/inc
                    stm32f10x_lib/inc
                    )

# Set toolchain options.
SET(M3_OPTS -mcpu=cortex-m3 -mthumb)

ADD_COMPILE_OPTIONS(${M3_OPTS}
                    -Wall
                    -Os
                    -fno-common
                    # Need in the line above: -fno-strict-aliasing OR -Wno-strict-aliasing
                    -fno-strict-aliasing
                    )

LIST(APPEND CMAKE_ASM_FLAGS "-c")

SET(LINKER_FLAGS
    ${M3_OPTS}
    -L"${PROJECT_SOURCE_DIR}/stm32f10x_lib"
    -L"${PROJECT_BINARY_DIR}"
    -nostdlib
    -Wl,--gc-sections,-Map=${MAP_FILE},-cref,-u,Reset_Handler
    "-T ${PROJECT_SOURCE_DIR}/stm32.ld"
    )

# Find source files.
FILE(GLOB_RECURSE ${PROJECT_NAME}_APP_SOURCES
     ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}_APP/**.c
)

FILE(GLOB_RECURSE ${PROJECT_NAME}_HW_SOURCES
     ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}_HW/**.c
)

# Gather libstm C source files.
FILE(GLOB_RECURSE LIBSTM_C_SOURCES
     ${PROJECT_SOURCE_DIR}/stm32f10x_lib/**.c
)

# Gather libstm ASM source files.
FILE(GLOB_RECURSE LIBSTM_ASM_SOURCES
     ${PROJECT_SOURCE_DIR}/stm32f10x_lib/**.s
)

# Exclude the files we dont need.
LIST(REMOVE_ITEM LIBSTM_C_SOURCES
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/misc.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_crc.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_dac.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_dbgmcu.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_fsmc.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_sdio.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_wwdg.c"
)

ADD_EXECUTABLE(${ELF_FILE} ${${PROJECT_NAME}_APP_SOURCES} ${${PROJECT_NAME}_HW_SOURCES})
ADD_DEPENDENCIES(${ELF_FILE} stm32)

TARGET_LINK_LIBRARIES(${ELF_FILE} ${LINKER_FLAGS} -lstm32)

# Create libstm32.a
ADD_LIBRARY(stm32 STATIC ${LIBSTM_C_SOURCES} ${LIBSTM_ASM_SOURCES})

# Generate the LSS file.
ADD_CUSTOM_COMMAND(
    OUTPUT ${LSS_FILE}
    COMMAND ${CMAKE_OBJDUMP} -h -S -C -D ${ELF_FILE} > ${LSS_FILE}
    DEPENDS ${ELF_FILE}
)

# Create the HEX file.
ADD_CUSTOM_COMMAND(
    OUTPUT ${HEX_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${ELF_FILE} ${HEX_FILE}
    DEPENDS ${ELF_FILE}
)

# Create the BIN file.
ADD_CUSTOM_COMMAND(
    OUTPUT ${BIN_FILE}
    COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_FILE} ${BIN_FILE}
    DEPENDS ${ELF_FILE}
)

# Print the ELF file size.
ADD_CUSTOM_COMMAND(
    OUTPUT print-size-${ELF_FILE}
    COMMAND ${CMAKE_SIZE} ${ELF_FILE}
    DEPENDS ${ELF_FILE}
)

# build the intel hex file for the device
ADD_CUSTOM_TARGET(
    ${PROJECT_NAME}
    ALL
    DEPENDS ${HEX_FILE} ${BIN_FILE} ${LSS_FILE} "print-size-${ELF_FILE}"
)
