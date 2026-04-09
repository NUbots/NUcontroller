#include "fan_controller.h"
#include <stdbool.h>
extern I2C_HandleTypeDef hi2c3;

uint8_t read_fan_register(uint8_t reg)
{
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c3, FAN_CONTROLLER_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}

uint8_t write_fan_register(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c3, FAN_CONTROLLER_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

bool fan_warning_state(uint8_t fan_id) {

    // Warning state is true if fan speed is less than or equal to the defined warning threshold
    return read_fan_speed(fan_id) <= FAN_RPM_WARNING;
}

void set_fan_pwm_freq(uint8_t freq)
{
    uint8_t control1 = read_fan_register(REG_CONTROL1);
    control1 &= ~(0b11 << 3);       // clear bits 3 and 4
    // Warning state is true if fan speed is less than or equal to the defined warning threshold
    return read_fan_speed(fan_id) <= FAN_RPM_WARNING;
    uint8_t control2 = read_fan_register(REG_CONTROL2);
    if (mode) {
        control2 |= (1 << 0); // set bit 0 to enable Direct Fan Control
    } else {
        control2 &= ~(1 << 0); // clear bit 0 to disable Direct Fan Control
    }
    write_fan_register(REG_CONTROL2, control2);
}

void set_fan_spin_up(bool enabled)
{
    uint8_t control2 = read_fan_register(REG_CONTROL2);
    if (enabled) {
        control2 |= (1 << 1); // set bit 1 to enable Spin-Up mode
    }
    else {
        control2 &= ~(1 << 1); // clear bit 1 to disable Spin-Up mode
    }
    write_fan_register(REG_CONTROL2, control2);
}

void set_fan_tachometer_enabled(uint8_t tachometer, bool enabled)
{
    uint8_t control3 = read_fan_register(REG_CONTROL3);
    if (enabled) {  
        control3 |= (1 << tachometer);  
    } 
    else {  
        control3 &= ~(1 << tachometer);  
    }
    write_fan_register(REG_CONTROL3, control3);
}

void set_fan_manual_pwm(uint8_t pwm_value) {
    write_fan_register(REG_PWMR, pwm_value);
}

uint16_t read_fan_speed(uint8_t tachometer) {
	uint8_t fan_register = (tachometer == 0 ? REG_FAN1COUNT : REG_FAN2COUNT);
    uint8_t msb = read_fan_register(fan_register);
	uint8_t lsb = read_fan_register(fan_register + 1);
	uint16_t fan_count = (msb << 8) | lsb;
	return 60 * 100000 / fan_count / PULSES_PER_REVOLUTION;
}

void fan_controller_init() {
    set_fan_mode(true);

    // enable both tachometers
    set_fan_tachometer_enabled(0, true);
    set_fan_tachometer_enabled(1, true);

    // set Direct Duty-Cycle Control Register to full blast.
    set_fan_manual_pwm(DEFAULT_FAN_SPEED);
}
