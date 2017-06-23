# Make sure the toolchain file has been supplied.
IF(NOT CMAKE_CROSSCOMPILING)
    IF(EXISTS "${CMAKE_SOURCE_DIR}/CM730.cmake")
        SET(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/CM730.cmake")
    ELSE()
        MESSAGE(FATAL_ERROR "This project must be cross-compiled. Unable to find toolchain file.")
    ENDIF()
ENDIF(NOT CMAKE_CROSSCOMPILING)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(CM730)

ENABLE_LANGUAGE(ASM)

# Set device options.
SET(DEVICE CM730 CACHE STRING "The device to target")
SET_PROPERTY(CACHE DEVICE PROPERTY STRINGS CM730 CM740)

# Set battery options.
SET(BATTERY_TYPE BATTERY_4CELL CACHE STRING "The type of battery to use")
SET_PROPERTY(CACHE BATTERY_TYPE PROPERTY STRINGS BATTERY_3CELL BATTERY_4CELL)

# BUILD_TYPE determines which files to build.
# ALL builds ELF, MAP, HEX, BIN, and LSS for the DEVICE and BATTERY_TYPE set by the user.
# CM730Bins builds only the BIN for both battery types for the CM730 device.
# CM740Bins builds only the BIN for both battery types for the CM740 device.
SET(BUILD_TYPE ALL CACHE STRING "The type of build to perform")
SET_PROPERTY(CACHE BUILD_TYPE PROPERTY STRINGS ALL CM730Bins CM740Bins)

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

# Set main compile output names.
SET(ELF_FILE ${PROJECT_NAME}.elf)
SET(MAP_FILE ${PROJECT_NAME}.map)

# Set linking flags.
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

# Create libstm32.a
ADD_LIBRARY(stm32 STATIC ${LIBSTM_C_SOURCES} ${LIBSTM_ASM_SOURCES})

IF(BUILD_TYPE STREQUAL "ALL")
    # Set project output files.
    SET(HEX_FILE ${PROJECT_NAME}.hex)
    SET(BIN_FILE ${PROJECT_NAME}.bin)
    SET(LSS_FILE ${PROJECT_NAME}.lss)

    ADD_EXECUTABLE(${ELF_FILE} ${${PROJECT_NAME}_APP_SOURCES} ${${PROJECT_NAME}_HW_SOURCES})
    ADD_DEPENDENCIES(${ELF_FILE} stm32)

    TARGET_COMPILE_DEFINITIONS(${ELF_FILE} PUBLIC -D${BATTERY_TYPE} -DFORCE_${DEVICE})

    TARGET_LINK_LIBRARIES(${ELF_FILE} ${LINKER_FLAGS} -lstm32)

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

ELSEIF(BUILD_TYPE STREQUAL "CM730Bins")
    FOREACH(BATTERY BATTERY_3CELL BATTERY_4CELL)
        SET(ELF_FILE CM730_${BATTERY}.elf)
        SET(BIN_FILE CM730_${BATTERY}.bin)

        ADD_EXECUTABLE(${ELF_FILE} ${${PROJECT_NAME}_APP_SOURCES} ${${PROJECT_NAME}_HW_SOURCES})
        ADD_DEPENDENCIES(${ELF_FILE} stm32)

        TARGET_COMPILE_DEFINITIONS(${ELF_FILE} PUBLIC -D${BATTERY} -DFORCE_CM730)

        TARGET_LINK_LIBRARIES(${ELF_FILE} ${LINKER_FLAGS} -lstm32)

        # Create the BIN file.
        ADD_CUSTOM_COMMAND(
            OUTPUT ${BIN_FILE}
            COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_FILE} ${BIN_FILE}
            DEPENDS ${ELF_FILE}
        )

        ADD_CUSTOM_TARGET(
            ${PROJECT_NAME}_${BATTERY}
            ALL
            DEPENDS ${BIN_FILE}
        )
    ENDFOREACH(BATTERY)

ELSEIF(BUILD_TYPE STREQUAL "CM740Bins")
    FOREACH(BATTERY BATTERY_3CELL BATTERY_4CELL)
        SET(ELF_FILE CM740_${BATTERY}.elf)
        SET(BIN_FILE CM740_${BATTERY}.bin)

        ADD_EXECUTABLE(${ELF_FILE} ${${PROJECT_NAME}_APP_SOURCES} ${${PROJECT_NAME}_HW_SOURCES})
        ADD_DEPENDENCIES(${ELF_FILE} stm32)

        TARGET_COMPILE_DEFINITIONS(${ELF_FILE} PUBLIC -D${BATTERY} -DFORCE_CM740)

        TARGET_LINK_LIBRARIES(${ELF_FILE} ${LINKER_FLAGS} -lstm32)

        # Create the BIN file.
        ADD_CUSTOM_COMMAND(
            OUTPUT ${BIN_FILE}
            COMMAND ${CMAKE_OBJCOPY} -O binary ${ELF_FILE} ${BIN_FILE}
            DEPENDS ${ELF_FILE}
        )

        ADD_CUSTOM_TARGET(
            ${PROJECT_NAME}_${BATTERY}
            ALL
            DEPENDS ${BIN_FILE}
        )
    ENDFOREACH(BATTERY)

ELSE()
    MESSAGE(FATAL_ERROR "Unknown build type '${BUILD_TYPE}'.")
ENDIF()
