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

uint16_t read_ADC_gain() {
    uint16_t gain = 0;

    uint8_t ADC_GAIN_1 = read_battery_register(REG_ADC_GAIN_1); // Read ADC_GAIN_1 register
    uint8_t ADC_GAIN_2 = read_battery_register(REG_ADC_GAIN_2); // Read ADC_GAIN_2 register

    gain = ((ADC_GAIN_1 & 0b1100) << 1) | ((ADC_GAIN_2 & 0b11100000) >> 5); // this mapping made sense in my head

    return 365 + (gain & 0b11111); // 365 + gain[4:0]
}

uint16_t read_cell_voltage(uint8_t cell_number) {
    if (cell_number < 1 || cell_number > 4) {
        return 0; // Invalid cell number
    }

    uint8_t raw_msb = read_battery_register(REG_CELL_1_H + (cell_number - 1) * 2); // Read MSB of the specified cell voltage register
    uint8_t raw_lsb = read_battery_register(REG_CELL_1_L + (cell_number - 1) * 2); // Read LSB of the specified cell voltage register

    uint16_t raw_cell = (((raw_msb & 0x3F) << 8) | raw_lsb);

    uint16_t gain = read_ADC_gain();

    int8_t offset = (int8_t)read_battery_register(REG_ADC_OFFSET);

    uint16_t cell_voltage_mV = (gain * raw_cell) / 1000 + offset;

    return cell_voltage_mV;
}

float read_battery_voltage() {
    uint16_t cell_1 = read_cell_voltage(1);
    uint16_t cell_2 = read_cell_voltage(2);
    uint16_t cell_3 = read_cell_voltage(3);
    uint16_t cell_4 = read_cell_voltage(4);
    
    float battery_voltage = (cell_1 + cell_2 + cell_3 + cell_4) / 1000.0f;

    return battery_voltage;
}