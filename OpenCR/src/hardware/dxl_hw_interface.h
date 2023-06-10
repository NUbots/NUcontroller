/*
 *  dxl_hw_interface.h
 *
 *  handling for the interface board on the back of the NUgus robot
 *
 *  Created on: 2023. 05. 16
 *      Author: Dexter Konijn
 */

#ifndef DXL_HW_INTERFACE_H
#define DXL_HW_INTERFACE_H

/**
 * Setup info:
 *
 *  - Buttons configured as pullups (internal)
 *  - LEDs are active low
 *
 *  - Button setup:
 *      - Interface board: 3 buttons
 *      - OpenCR:          4 buttons
 *      - HardwareIO:      3 buttons
 *
 *      - Interface board (by schematic):
 *          [Black, Green, Red] = [S3, S2, S1]
 *
 *  - Button control:
 *      - dxl_node_op3_btn_loop() handles dynamixel power disable
 *          - Uses PIN_BUTTON_S4 currently (Pin 59)
 *
 *  - LED setup:
 *      - Interface board: 2 RGB and 5 normal
 *      - OpenCR:          1 RGB and 3 normal
 *      - HardwareIO:      2 RGB and 3 normal - Only 1 RGB is set (because OpenCR)
 * - LED control:
 *      [All in dxl_hw_op3.cpp]
 *      - dxl_hw_op3_led_set(uint8_t pin_num, uint8_t value)
 *          > does a direct write to the pin
 *      - dxl_node_write_byte(uint16_t addr, uint8_t data)
 *          > checks if LEDs were written to and turns them on if yes
 *          > LED_1..3 set from LSB->MSB (p_dxl_mem->LED & (1 << 2) sets LED_3)
 *      - dxl_hw_op3_led_pwm(uint8_t pin_num, uint8_t value)
 *          > sets global for PWM value
 *      - handler_led(void)
 *          > Triggered by interrupt, sets values based on globals
 *
 */

/// @todo Fill these in with correct pins as per OpenCR schematic once Clayton
///       knows what pins go where.

/**
 * https://github.com/ROBOTIS-GIT/OpenCR-Hardware/blob/master/Schematic/OpenCR_REVH.pdf
 *  - Will be in the range of TEST_PIN_1..12
 *      - TEST_PIN_1  -> 69  (PB10)
 *      - TEST_PIN_2  -> 70  (PB11)
 *      - TEST_PIN_3  -> 7   (PC13-ANTI_TAMP) -- anti tamper functionality is default off
 *      - TEST_PIN_4  -> 116 (PD2)
 *      - TEST_PIN_5  -> 2   (PE3)
 *      - TEST_PIN_6  -> 87  (PG2)
 *      - TEST_PIN_7  -> 63  (PE10)
 *           ...         ⋮    ⋮
 *      - TEST_PIN_12 -> 68  (PE15)
 *
 * https://github.com/ROBOTIS-GIT/OpenCR/blob/master/arduino/opencr_arduino/opencr/variants/OpenCR/chip.h
 *  - What??
 *      - Physical OpenCR silkscreen has GPIO 1..20
 *      - Schematic has these connected to TEST_PIN1..18
 *      - Schematic then links these to CPU pin 69,70,7,116,2,87..68,10-12,77,14,78
 *      - OpenCR/chip.h then has macros BDPIN_GPIO_1..18
 *      - OpenCR/chip.h then links these to pin number 50..67
 *
 *      So... taking chip.h as the correct definitions (used elsewhere in fw)
 *          - The defaults actually were
 *              PIN_LED_R       BDPIN_GPIO_1    Silkscreen GPIO 3
 *              PIN_LED_G       BDPIN_GPIO_2    Silkscreen GPIO 4
 *              PIN_LED_B       BDPIN_GPIO_3    Silkscreen GPIO 5
 *              PIN_LED_1       BDPIN_GPIO_4    Silkscreen GPIO 6
 *              PIN_LED_2       BDPIN_GPIO_5    Silkscreen GPIO 7
 *              PIN_LED_3       BDPIN_GPIO_6    Silkscreen GPIO 8
 *              PIN_BUTTON_S1   BDPIN_GPIO_7    Silkscreen GPIO 9
 *              PIN_BUTTON_S2   BDPIN_GPIO_8    Silkscreen GPIO 10
 *              PIN_BUTTON_S3   BDPIN_GPIO_9    Silkscreen GPIO 11
 *              PIN_BUTTON_S4   BDPIN_GPIO_10   Silkscreen GPIO 12
 *
 *          - Hardware requirements for interface board are
 *              Use silkscreen GPIO 3..12 (TEST_PIN1..10)
 *              S1 -> Silkscreen GPIO 9 (TEST_PIN7)
 *              S2 -> Silkscreen GPIO 10 (TEST_PIN8)
 *              S3 -> Silkscreen GPIO 11 (TEST_PIN9)
 *              Short Silkscreen GPIO 9 and 12 (TEST_PIN7 and TEST_PIN_10)
 */


#define HW_INTERFACE_BUTTON_RED   50
#define HW_INTERFACE_BUTTON_GREEN 51
#define HW_INTERFACE_BUTTON_BLACK 52

#define HW_INTERFACE_LED_1 53
#define HW_INTERFACE_LED_2 54
#define HW_INTERFACE_LED_3 55

#define HW_INTERFACE_LED_R 56
#define HW_INTERFACE_LED_G 57
#define HW_INTERFACE_LED_B 58

#endif  // DXL_HW_INTERFACE_H