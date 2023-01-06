/*
 *  dxl_hw_op3.h
 *
 *  dynamixel hardware op3
 *
 *  Created on: 2016. 10. 21.
 *      Author: Baram
 */

#ifndef DXL_HW_OP3_H
#define DXL_HW_OP3_H


#include "dxl_def.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

/* Voltage levels from CM730 repo
// Set whether a 3 cell (11.1V) or 4 cell (14.8V) LiPo battery is being used
// Should be specified (either BATTERY_4CELL or BATTERY_3CELL) on the command line.

// Define the battery voltage levels
#if BATTERY_4CELL
#define VOLTAGE_LEVEL_6             172  // 17.2V Overcharged
#define VOLTAGE_LEVEL_5             168  // 16.8V Fully charged
#define VOLTAGE_LEVEL_4             159  // 15.9V      ...
#define VOLTAGE_LEVEL_3             155  // 15.5V      ...
#define VOLTAGE_LEVEL_2             151  // 15.1V      ...
#define VOLTAGE_LEVEL_1             147  // 14.7V Warning level
#define VOLTAGE_LEVEL_0             141  // 14.1V Empty
#elif BATTERY_3CELL
#define VOLTAGE_LEVEL_6             130  // 13.0V Overcharged
#define VOLTAGE_LEVEL_5             126  // 12.6V Fully charged
#define VOLTAGE_LEVEL_4             119  // 11.9V      ...
#define VOLTAGE_LEVEL_3             116  // 11.6V      ...
#define VOLTAGE_LEVEL_2             113  // 11.3V      ...
#define VOLTAGE_LEVEL_1             110  // 11.0V Warning level
#define VOLTAGE_LEVEL_0             106  // 10.6V Empty
#else
#error "Invalid battery specification => Must set either -DBATTERY_4CELL or -DBATTERY_3CELL"
#endif

#define LOW_BATTERY_LIMIT           VOLTAGE_LEVEL_1
#define HIGH_BATTERY_LIMIT          VOLTAGE_LEVEL_6
*/

#define PIN_LED_R 50
#define PIN_LED_G 51
#define PIN_LED_B 52

#define PIN_LED_1 53
#define PIN_LED_2 54
#define PIN_LED_3 55

#define PIN_BUTTON_S1 56
#define PIN_BUTTON_S2 57
#define PIN_BUTTON_S3 58
#define PIN_BUTTON_S4 59


void dxl_hw_op3_init(void);
void dxl_hw_op3_update(void);

// button
uint8_t dxl_hw_op3_button_read(uint8_t pin_num);

// led
void dxl_hw_op3_led_set(uint8_t pin_num, uint8_t value);
void dxl_hw_op3_led_pwm(uint8_t pin_num, uint8_t value);

// voltage
uint8_t dxl_hw_op3_voltage_read(void);

// acc
int16_t dxl_hw_op3_gyro_get_x(void);
int16_t dxl_hw_op3_gyro_get_y(void);
int16_t dxl_hw_op3_gyro_get_z(void);

int16_t dxl_hw_op3_acc_get_x(void);
int16_t dxl_hw_op3_acc_get_y(void);
int16_t dxl_hw_op3_acc_get_z(void);

int16_t dxl_hw_op3_get_rpy(uint8_t rpy);
void dxl_hw_op3_start_cali(uint8_t index);
int16_t dxl_hw_op3_get_cali(uint8_t index);
void dxl_hw_op3_clear_cali(uint8_t index);


void dxl_hw_op3_set_offset(uint8_t index, float offset_data);
float dxl_hw_op3_get_offset(uint8_t index);

void dxl_hw_op3_start_gyro_cali(void);
bool dxl_hw_op3_get_gyro_cali_done(void);


#endif
