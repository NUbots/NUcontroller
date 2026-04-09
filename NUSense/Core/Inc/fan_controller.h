#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

// Fan configuration
#define PULSES_PER_REVOLUTION 4 // Fan tachometer pulse count that corresponds to one revolution
#define FAN_RPM_WARNING 100 // RPM

// Fan Controller I2C Address
#define FAN_CONTROLLER_ADDRESS 0xA0
#define DEFAULT_FAN_SPEED 0xFF // full steam ahead

// Registers
#define REG_CONTROL1 0x00
#define REG_CONTROL2 0x01
#define REG_CONTROL3 0x02
#define REG_FAN1COUNT 0x52 // MSB of 2 byte fan controller tachometer count for fan 1
#define REG_FAN2COUNT 0x54 // MSB of 2 byte fan controller tachometer count for fan 2
#define REG_PWMR     0x50
#define REG_STATUS   0x5A

// Functions
void fan_controller_init();

/// @brief Reads a value from a fan controller register.
/// @param reg The register address to read from.
/// @return The value read from the register.
uint8_t read_fan_register(uint8_t reg);

/// @brief Writes a value to a fan controller register.
/// @param reg The register address to write to.
/// @param value The value to write.
/// @return The HAL status of the write operation.
uint8_t write_fan_register(uint8_t reg, uint8_t value);

/// @brief Checks the fan warning states for the specified fan.
/// @param fan_id The ID of the fan to check (0 for Fan 1 (J401 on NUSense), 1 for Fan 2 (J402 on NUSense)) 
/// @return The warning state of the specified fan. Returns true if there is a warning, false if there is no warning.
bool fan_warning_state(uint8_t fan_id);

/// @brief Sets the PWM frequency of the fan. The frequency is determined by bits 3 and 4 of Control Register 1.
/// @param freq The desired frequency setting (0b00 for 33Hz, 0b01 for 150Hz, 0b10 for 1500Hz, 0b11 for 25kHz)
void set_fan_pwm_freq(uint8_t freq);

/// @brief Sets the fan mode between manual and automatic.
/// @param mode true for manual mode (Direct Fan Control enabled), false for automatic mode (Direct Fan Control disabled)
void set_fan_mode(bool mode);

/// @brief Sets the fan spin-up mode.
/// @param enabled true to enable spin-up mode, false to disable
void set_fan_spin_up(bool enabled);

/// @brief Sets the manual PWM value for the fan.
/// @param pwm_value
void set_fan_manual_pwm(uint8_t pwm_value);

/// @brief reads the fan speed from the controller
/// @param tachometer 0 for Fan 1, 1 for Fan 2
/// @return The speed in RPM.
uint16_t read_fan_speed(uint8_t tachometer);

/// @brief Enables or disables the tachometer for the specified fan.
/// @param tachometer  0 for Tachometer 1, 1 for Tachometer 2
/// @param enabled true to enable, false to disable
void set_fan_tachometer_enabled(uint8_t tachometer, bool enabled);

#ifdef __cplusplus
}
#endif

#endif // FAN_CONTROLLER_H
