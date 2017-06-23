# Make sure the toolchain file has been supplied.
IF(NOT CMAKE_CROSSCOMPILING)
    IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CM730.cmake")
        SET(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CM730.cmake")
    ELSE()
        MESSAGE(FATAL_ERROR "This project must be cross-compiled. Unable to find toolchain file.")
    ENDIF()
ENDIF(NOT CMAKE_CROSSCOMPILING)

# Hide some useless cmake variables
MARK_AS_ADVANCED(CMAKE_INSTALL_PREFIX)

CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(CM730 C ASM)

# Set device options.
SET(AVAILABLE_DEVICES CM730 CM740 CACHE STRING "The devices to target (CM730 and/or CM740")
MARK_AS_ADVANCED(AVAILABLE_DEVICES)

# Set battery options.
SET(AVAILBLE_BATTERIES 3CELL 4CELL CACHE STRING "The type of battery to use (3CELL and/or 4CELL)")
MARK_AS_ADVANCED(AVAILBLE_BATTERIES)

FOREACH(battery ${AVAILBLE_BATTERIES})
    OPTION(BATTERY_${battery} "Build a variant with a ${battery} battery" ON)
ENDFOREACH(battery)

FOREACH(device ${AVAILABLE_DEVICES})
    OPTION(DEVICE_${device} "Build a variant for the ${device} platform" ON)
ENDFOREACH(device)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/CM730_HW/inc
                    ${PROJECT_SOURCE_DIR}/CM730_APP/inc
                    ${PROJECT_SOURCE_DIR}/stm32f10x_lib/inc
)

# Set toolchain options.
SET(M3_OPTS -mcpu=cortex-m3 -mthumb)

ADD_COMPILE_OPTIONS(${M3_OPTS}
                    -Wall
                    -std=c11
                    -Os
                    -fno-common
                    # Need in the line above: -fno-strict-aliasing OR -Wno-strict-aliasing
                    -fno-strict-aliasing
)

LIST(APPEND CMAKE_ASM_FLAGS "-c")

# Set linking flags.
SET(linker_flags
    ${M3_OPTS}
    -L"${PROJECT_SOURCE_DIR}/stm32f10x_lib"
    -nostdlib
    -Wl,--gc-sections,-Map=${PROJECT_NAME}.map,-cref,-u,Reset_Handler
    "-T ${PROJECT_SOURCE_DIR}/stm32.ld"
)

# Find our source files.
FILE(GLOB_RECURSE src
     ${PROJECT_SOURCE_DIR}/CM730_APP/**.c
     ${PROJECT_SOURCE_DIR}/CM730_HW/**.c
)

# Gather libstm source files.
FILE(GLOB_RECURSE stm_src
     ${PROJECT_SOURCE_DIR}/stm32f10x_lib/**.c
     ${PROJECT_SOURCE_DIR}/stm32f10x_lib/**.s
)

# Exclude the files we dont need.
LIST(REMOVE_ITEM stm_src
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/misc.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_crc.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_dac.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_dbgmcu.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_fsmc.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_sdio.c"
     "${PROJECT_SOURCE_DIR}/stm32f10x_lib/src/stm32f10x_wwdg.c"
)

# Create libstm32.a
ADD_LIBRARY(stm32 STATIC ${stm_src})

# Go through the combinations that we need to build
FOREACH(battery ${AVAILBLE_BATTERIES})
    FOREACH(device ${AVAILABLE_DEVICES})
        # Build our selected configurations
        IF(BATTERY_${battery} AND DEVICE_${device})

            # Make our output files lowercase
            SET(target ${device}_${battery})
            STRING(TOLOWER ${target} target)

            SET(bin ${target}.bin)

            # Create our elf target
            ADD_EXECUTABLE(${target} ${src})
            SET_TARGET_PROPERTIES(${target} PROPERTIES OUTPUT_NAME ${target}.elf)
            TARGET_COMPILE_DEFINITIONS(${target} PUBLIC -DBATTERY_${battery} -DFORCE_${device})

            TARGET_LINK_LIBRARIES(${target} stm32 ${linker_flags})

            # Create the BIN file.
            ADD_CUSTOM_COMMAND(
                OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${bin}
                COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${target}> ${CMAKE_CURRENT_BINARY_DIR}/${bin}
                DEPENDS ${target}
            )

            # Add to our output list
            LIST(APPEND bin_files ${CMAKE_CURRENT_BINARY_DIR}/${bin})

        ENDIF()
    ENDFOREACH(device)
ENDFOREACH(battery)

# Make the bin file a dependency of all
ADD_CUSTOM_TARGET(
    bins
    ALL
    DEPENDS ${bin_files}
)
