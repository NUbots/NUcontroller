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

#define HW_INTERFACE_BUTTON_RED   0
#define HW_INTERFACE_BUTTON_GREEN 0
#define HW_INTERFACE_BUTTON_BLACK 0

#define HW_INTERFACE_LED_1 0
#define HW_INTERFACE_LED_2 0
#define HW_INTERFACE_LED_3 0

#define HW_INTERFACE_LED_R 0
#define HW_INTERFACE_LED_G 0
#define HW_INTERFACE_LED_B 0

#endif  // DXL_HW_INTERFACE_H