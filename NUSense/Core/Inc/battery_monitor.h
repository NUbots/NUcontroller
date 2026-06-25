#ifndef battery_monitor_h
#define battery_monitor_h

#include "main.h"

// Battery Monitor I2C Address
#define BATTERY_MONITOR_ADDRESS 0x08

#define REG_SYS_CTRL1 0x04

#define REG_ADC_GAIN_1 0x50
#define REG_ADC_GAIN_2 0x59
#define REG_ADC_OFFSET 0x51

#define REG_CELL_1_H 0x0C // MSB of 2 byte cell voltage register for cell 1
#define REG_CELL_1_L 0x0D // et cetera
#define REG_CELL_2_H 0x0E // et cetera
#define REG_CELL_2_L 0x0F
#define REG_CELL_3_H 0x10
#define REG_CELL_3_L 0x11
#define REG_CELL_4_H 0x12
#define REG_CELL_4_L 0x13

/// @brief Initialises the battery monitor IC: the BQ76920 over I2C3.
void battery_monitor_init();

/// @brief Reads a register from the battery monitor IC.
/// @param reg The register to read.
/// @return The value of the register.
uint8_t read_battery_register(uint8_t reg);

/// @brief Writes a value to a register in the battery monitor IC.
/// @param reg The register to write to.
/// @param value The value to write.
/// @return HAL status.
uint8_t write_battery_register(uint8_t reg, uint8_t value);

/// @brief reads the ADC gain from the battery monitor IC
/// @return the value of the ADC gain (uV/LSB)
uint16_t read_ADC_gain();

/// @brief reads the voltage of a specific cell from the battery monitor IC
/// @param cell_number The number of the cell to read (1-4)
/// @return the voltage of the specified cell (mV)
uint16_t read_cell_voltage(uint8_t cell_number);

/// @brief gets the whole 4S battery voltage
/// @return the battery voltage in Volts.
float read_battery_voltage();

#endif