#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// Fan Controller I2C Address
#define FAN_CONTROLLER_ADDRESS 0xA0

// Registers
#define REG_CONTROL1 0x00
#define REG_CONTROL2 0x01
#define REG_CONTROL3 0x02
#define REG_PWMR     0x50
#define REG_STATUS   0x5A

// Functions
void fan_controller_init();
uint8_t read_fan_register(uint8_t reg);
uint8_t write_fan_register(uint8_t reg, uint8_t value);
void set_fan_pwm_freq(uint8_t freq);
void set_fan_mode(bool mode);
void set_fan_spin_up(bool enabled);
void set_fan_manual_pwm(uint8_t pwm_value);
void set_fan_tachometer_enabled(uint8_t tachometer, bool enabled);

#endif // FAN_CONTROLLER_H