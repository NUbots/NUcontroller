#include <battery_monitor.h>
extern I2C_HandleTypeDef hi2c3;

uint8_t read_battery_register(uint8_t reg) {
    uint8_t value = 0;
    HAL_I2C_Mem_Read(&hi2c3, BATTERY_MONITOR_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
    return value;
}

uint8_t write_battery_register(uint8_t reg, uint8_t value) {
    return HAL_I2C_Mem_Write(&hi2c3, BATTERY_MONITOR_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, HAL_MAX_DELAY);
}

void battery_monitor_init() {
    uint8_t sys_ctrl1 = read_battery_register(REG_SYS_CTRL1); // Read SYS_CTRL1 register
    sys_ctrl1 |= 0x04; // Set bit 4 for ADC_EN (enable ADC)
    write_battery_register(REG_SYS_CTRL1, sys_ctrl1); // Write back to SYS_CTRL1 register
}