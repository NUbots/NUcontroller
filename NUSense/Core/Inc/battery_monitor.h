#ifndef battery_monitor_h
#define battery_monitor_h

#include "main.h"

// Battery Monitor I2C Address
#define BATTERY_MONITOR_ADDRESS 0x08

#define REG_SYS_CTRL1 0x04

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

#endif