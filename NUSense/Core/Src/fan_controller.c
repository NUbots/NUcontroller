#include "fan_controller.h"
extern I2C_HandleTypeDef hi2c3;

/// @brief Reads a value from a fan controller register.
/// @param reg The register address to read from.
/// @return The value read from the register.
uint8_t read_fan_register(uint8_t reg)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c3, FAN_CONTROLLER_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}

/// @brief Writes a value to a fan controller register.
/// @param reg The register address to write to.
/// @param value The value to write.
/// @return The HAL status of the write operation.
uint8_t write_fan_register(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c3, FAN_CONTROLLER_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

/// @brief Sets the PWM frequency of the fan. The frequency is determined by bits 3 and 4 of Control Register 1.
/// @param freq The desired frequency setting (0b00 for 33Hz, 0b01 for 150Hz, 0b10 for 1500Hz, 0b11 for 25kHz)
void set_fan_pwm_freq(uint8_t freq)
{
    uint8_t control1 = read_fan_register(REG_CONTROL1);
    control1 &= ~(0b11 << 3);       // clear bits 3 and 4
    control1 |= (freq & 0b11) << 3; // set bits 3 and 4 to the desired frequency
    write_fan_register(REG_CONTROL1, control1);
}

/// @brief Sets the fan mode between manual and automatic.
/// @param mode true for manual mode (Direct Fan Control enabled), false for automatic mode (Direct Fan Control disabled)
void set_fan_mode(bool mode)
{
    uint8_t control2 = read_fan_register(REG_CONTROL2);
    if (mode) {
        control2 |= (1 << 0); // set bit 0 to enable Direct Fan Control
    } else {
        control2 &= ~(1 << 0); // clear bit 0 to disable Direct Fan Control
    }
    write_fan_register(REG_CONTROL2, control2);
}

/// @brief Sets the fan spin-up mode.
/// @param enabled true to enable spin-up mode, false to disable
void set_fan_spin_up(bool enabled)
{
    uint8_t control2 = read_fan_register(REG_CONTROL2);
    if (enabled) {
        control2 |= (1 << 1); // set bit 1 to enable Spin-Up mode
    } else {
        control2 &= ~(1 << 1); // clear bit 1 to disable Spin-Up mode
    }
    write_fan_register(REG_CONTROL2, control2);
}

/// @brief Enables or disables the tachometer for the specified fan.
/// @param tachometer  0 for Tachometer 1, 1 for Tachometer 2
/// @param enabled true to enable, false to disable
void set_fan_tachometer_enabled(uint8_t tachometer, bool enabled)
{
    uint8_t control3 = read_fan_register(REG_CONTROL3);
    if (tachometer == 0) {
        if (enabled) {
            control3 |= (1 << 0); // set bit 0 to enable Tachometer 1
        } else {
            control3 &= ~(1 << 0); // clear bit 0 to disable Tachometer 1
        }
    } else if (tachometer == 1) {
        if (enabled) {
            control3 |= (1 << 1); // set bit 1 to enable Tachometer 2
        } else {
            control3 &= ~(1 << 1); // clear bit 1 to disable Tachometer 2
        }
    }
    write_fan_register(REG_CONTROL3, control3);
}

/// @brief Sets the manual PWM value for the fan.
/// @param pwm_value
void set_fan_manual_pwm(uint8_t pwm_value)
{
    write_fan_register(REG_PWMR, pwm_value);
}

/// @brief Sets up the Fan Controller with default settings: 1500Hz PWM frequency, manual mode enabled, tachometers disabled, and ~78% duty cycle.
void fan_controller_init()
{
    // set the PWM frequency to 1500Hz (0b10)
    set_fan_pwm_freq(0b10);

    // put the fan into manual mode
    set_fan_mode(true);

    // disable tachometers
    set_fan_tachometer_enabled(0, false);
    set_fan_tachometer_enabled(1, false);

    // set Direct Duty-Cycle Control Register to full blast.
    set_fan_manual_pwm(255);
}