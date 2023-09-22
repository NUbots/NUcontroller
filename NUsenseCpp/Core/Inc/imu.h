/***********************************************************/
/*
 * NUbots NUfsr IMU command table
 * Author: Benjamin Young
 * File: Header, Defines all IMU commands and data options
 * for ICM-20689.
 *
 * Addendum:
 * Clayton Carlon
 * 9/9/2022
 * Ported from NUfsr to NUsense for testing.
 *
 ***********************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _IMU_H_
#define _IMU_H_

/* Includes */
#include "main.h"
#include "spi.h"
#include "settings.h"

//---------------Helpful Defines---------------------//

// Masks for the flags for each SPI interface, in this case only one:
#define SPI4_RX 0x40U
#define SPI4_TX 0x80U

//                                    /* 7 6 5 4 3 2 1 0 */
#define BIT_0 0x01                    /* 0 0 0 0 0 0 0 1 -> 0x01 */
#define BIT_1 0x02                    /* 0 0 0 0 0 0 1 0 -> 0x02 */
#define BIT_2 0x04                    /* 0 0 0 0 0 1 0 0 -> 0x04 */
#define BIT_3 0x08                    /* 0 0 0 0 1 0 0 0 -> 0x08 */
#define BIT_4 0x10                    /* 0 0 0 1 0 0 0 0 -> 0x10 */
#define BIT_5 0x20                    /* 0 0 1 0 0 0 0 0 -> 0x20 */
#define BIT_6 0x40                    /* 0 1 0 0 0 0 0 0 -> 0x40 */
#define BIT_7 0x80                    /* 1 0 0 0 0 0 0 0 -> 0x80 */
#define BLANK 0x00

// From Table-1, Table-2, and Table-3 from the datasheet.
#define ACCEL_SENSITIVTY_2G            16384     // LSB/g
#define ACCEL_SENSITIVTY_4G            8192     // LSB/g
#define ACCEL_SENSITIVTY_8G            4096     // LSB/g
#define ACCEL_SENSITIVTY_16G        2048     // LSB/g
#define TEMP_SENSITIVITY            326.8    // LSB/degC
#define GYRO_SENSITIVITY_250DPS        131        // LSB/(deg/s)
#define GYRO_SENSITIVITY_500DPS        65.5    // LSB/(deg/s)
#define GYRO_SENSITIVITY_1000DPS    32.8    // LSB/(deg/s)
#define GYRO_SENSITIVITY_2000DPS    16.4    // LSB/(deg/s)

//---------------Address Byte Defines----------------//
/* Common Defines */
#define IMU_READ 0x80
#define IMU_WRITE 0x00

/* Command Defines */                /* ADR Register Function */
// Self Test
#define SELF_TEST_X_GYRO 0x00    /* Gyro X Self test Reg */
#define SELF_TEST_Y_GYRO 0x01    /* Gyro Y Self test Reg */
#define SELF_TEST_Z_GYRO 0x02    /* Gyro Z Self test Reg */
#define SELF_TEST_X_ACCEL 0x0D/* Accel X Self test Reg */
#define SELF_TEST_Y_ACCEL 0x0E/* Accel Y Self test Reg */
#define SELF_TEST_Z_ACCEL 0x0F/* Accel Z Self test Reg */
// Offset adjustment
#define XG_OFFS_USRH 0x13        /* Gyro X Offset Adjustment Reg 15:8 */
#define XG_OFFS_USRL 0x14        /* Gyro X Offset Adjustment Reg 7:0 */
#define YG_OFFS_USRH 0x15        /* Gyro Y Offset Adjustment Reg 15:8 */
#define YG_OFFS_USRL 0x16        /* Gyro Y Offset Adjustment Reg 7:0 */
#define ZG_OFFS_USRH 0x17        /* Gyro Z Offset Adjustment Reg 15:8 */
#define ZG_OFFS_USRL 0x18        /* Gyro Z Offset Adjustment Reg 7:0 */
// Configuration
#define SMPLRT_DIV 0x19            /* Sample Rate divider */
#define CONFIG 0x1A                 /* Configuration */
#define GYRO_CONFIG 0x1B             /* Gyro Configuration */
#define ACCEL_CONFIG 0x1C             /* Accel Configuration */
#define ACCEL_CONFIG2 0x1D         /* Accel Configuration */
#define LP_MODE_CFG 0x1E             /* Low Power Mode Config */
// Interrupt Configuration
#define ACCEL_WOM_X_THR 0x20        /* X-Axis Accel Wake-On Motion Threshold */
#define ACCEL_WOM_Y_THR 0x21        /* Y-Axis Accel Wake-On Motion Threshold */
#define ACCEL_WOM_Z_THR 0x22        /* Z-Axis Accel Wake-On Motion Threshold */
#define FIFO_EN 0x23                /* FIFO Enable */
#define FSYNC_INT 0x36            /* FSYNC Interrupt Status */
#define INT_PIN_CFG 0x37            /* INT/DRDY PIN / BYPASS Enable Config */
#define INT_ENABLE 0x38            /* Interrupt Enable */
#define DMP_INT_STATUS 0x39        /* DMP Interrupt Status */
#define INT_STATUS 0x3A            /* Interrupt Status */
// Accelerometer Measurements
#define ACCEL_XOUT_H 0x3B         /* X_Accel High byte */
#define ACCEL_XOUT_L 0x3C         /* X_Accel Low byte */
#define ACCEL_YOUT_H 0x3D         /* Y_Accel High byte */
#define ACCEL_YOUT_L 0x3E         /* Y_Accel Low byte */
#define ACCEL_ZOUT_H 0x3F         /* Z_Accel High byte */
#define ACCEL_ZOUT_L 0x40            /* Z_Accel Low byte */
// Temperature Measurements
#define TEMP_OUT_H 0x41            /* Temp High Byte */
#define TEMP_OUT_L 0x42            /* Temp Low Byte */
// Gyroscope Measurements
#define GYRO_XOUT_H 0x43            /* X_GYRO High byte */
#define GYRO_XOUT_L 0x44             /* X_GYRO Low byte */
#define GYRO_YOUT_H 0x45             /* Y_GYRO High byte */
#define GYRO_YOUT_L 0x46             /* Y_GYRO Low byte */
#define GYRO_ZOUT_H 0x47             /* Z_GYRO High byte */
#define GYRO_ZOUT_L 0x48             /* Z_GYRO Low byte */
// Additional Options/Configurations
#define SIGNAL_PATH_RESET 0x68     /* Signal Path Reset */
#define ACCEL_INTEL_CTRL 0x69     /* Accel Intelligence Control */
#define USER_CTRL 0x6A             /* User Control */
// Power Management
#define PWR_MGMT_1 0x6B             /* Power Management 1 */
#define PWR_MGMT_2 0x6C             /* Power Management 2 */
// FIFO Commands
#define FIFO_COUNTH 0x72            /* High FIFO count byte */
#define FIFO_COUNTL 0x73            /* LOW FIFO count byte */
#define FIFO_R_W 0x74             /* FIFO buffer Read/Write 8 */
// WHO AM I
#define WHO_AM_I 0x75             /* Device Information Register */
// ACCEL Offset Reg
#define XA_OFFSET_H 0x77             /* X_Accel High byte offset calculation */
#define XA_OFFSET_L 0x78             /* X_Accel Low byte offset calculation */
#define YA_OFFSET_H 0x7A             /* Y_Accel High byte offset calculation */
#define YA_OFFSET_L 0x7B             /* Y_Accel Low byte offset calculation */
#define ZA_OFFSET_H 0x7D             /* Z_Accel High byte offset calculation */
#define ZA_OFFSET_L 0x7E             /* Z_Accel Low byte offset calculation */

//-------------------------------------------
// Self-Test R/W
/*
 * The value in this register indicates the self-test output generated
 *    during manufacturing tests. This value is to be used to check
 *    against subsequent self-test outputs performed by the end user
 */

//-------------------------------------------
// GYRO Offset adjustment R/W
/*
 * This register is used to remove DC bias from the sensor output. The value in
 * this register is added to the gyroscope sensor value before going into
 * the sensor register.
 */

//-------------------------------------------
// Sample rate Divider R/W
/*
 * Divides the internal sample rate (see register CONFIG) to generate the sample
 * rate that controls sensor data output rate, FIFO sample rate.
 * SAMPLE_RATE = INTERNAL_SAMPLE_RATE / (1 + SMPLRT_DIV)
 * Where INTERNAL_SAMPLE_RATE = 1kHz
 */

//-------------------------------------------
// Configuration R/W
/*
 * 7: Always set to 0
 * 6: When set to ‘1’, when the FIFO is full, additional writes will not be written to FIFO.
 *     When set to ‘0’, when the FIFO is full, additional writes will be written to the FIFO,
 *     replacing the oldest data.
 * 5-3: Enables the FSYNC pin data to be sampled (Pin is held at 0 by NUfsr design).
 * 2-0: Digital Low Pas Filter Config: Refer to data sheet page 39
 */
// 6:
#define CONFIG_FIFO_MODE_OVERFLOW_WAIT BIT_6

//-------------------------------------------
// GYRO Configuration R/W
/*
 * 7-5: Gyro self-test
 * 4-3: Gyro Full Scale select
 * 2: RESERVED
 * 1-0: FCHOICE_B
 */
// 7-5
#define GYRO_CONFIG_X_SELF_TEST BIT_7
#define GYRO_CONFIG_Y_SELF_TEST BIT_6
#define GYRO_CONFIG_Z_SELF_TEST BIT_5
// 4-3
#define GYRO_CONFIG_FS_SEL_250DPS 0x00
#define GYRO_CONFIG_FS_SEL_500DPS BIT_3
#define GYRO_CONFIG_FS_SEL_1000DPS BIT_4
#define GYRO_CONFIG_FS_SEL_2000DPS 0x18
// 1-0
// Refer to IMU data-sheet

//-------------------------------------------
// ACCEL Configuration1 R/W
/*
 * 7-5: ACCEL self-test
 * 4-3: ACCEL Full Scale select
 * 2-0: RESERVED
 */
// 7-5
#define ACCEL_CONFIG1_X_SELF_TEST BIT_7
#define ACCEL_CONFIG1_Y_SELF_TEST BIT_6
#define ACCEL_CONFIG1_Z_SELF_TEST BIT_5
// 4-3
#define ACCEL_CONFIG1_FS_SEL_2G 0x00
#define ACCEL_CONFIG1_FS_SEL_4G BIT_3
#define ACCEL_CONFIG1_FS_SEL_8G BIT_4
#define ACCEL_CONFIG1_FS_SEL_16G 0x18

//-------------------------------------------
// ACCEL Configuration2 R/W
/*
 * 7-6: FIFO Size
 * 5-4: Averaging filter settings for Low Power Accelerometer mode:
 * 3: FCHOICE_B: Used to bypass DLPF.
 * 2-0: Accelerometer low pass filter setting.
 */
// 7-6
#define ACCEL_CONFIG2_FIFO_SIZE_512B 0x00
#define ACCEL_CONFIG2_FIFO_SIZE_1KB BIT_6
#define ACCEL_CONFIG2_FIFO_SIZE_2KB BIT_7
#define ACCEL_CONFIG2_FIFO_SIZE_4KB 0xC0
// 5-4
#define ACCEL_CONFIG2_DEC2_CFG_4SAMPLES 0x00
#define ACCEL_CONFIG2_DEC2_CFG_8SAMPLES BIT_4
#define ACCEL_CONFIG2_DEC2_CFG_16SAMPLES BIT_5
#define ACCEL_CONFIG2_DEC2_CFG_32SAMPLES 0x30
// 3
#define ACCEL_CONFIG2_ACCEL_FCHOICE_B_TRUE BIT_3
#define ACCEL_CONFIG2_ACCEL_FCHOICE_B_FALSE 0x00
// 2-0
// Refer to data-sheet page 41

//-------------------------------------------
// Low power mode config R/W
/*
 * 7: When set to ‘1’ low-power gyroscope mode is enabled. Default
 *    setting is ‘0’
 * 6-4: Averaging filter configuration for low-power gyroscope mode.
 *  Default setting is ‘000’. Refer to data-sheet page 42
 * 3-0: RESERVED
 */
// 7
#define LP_MODE_CFG_GYRO_CYCLE_TRUE BIT_7

//-------------------------------------------
// Wake on motion threshold R/W
/*
 * 7-0: Wake on Motion Interrupt threshold for respective axis accelerometer.
 */

//-------------------------------------------
// FIFO Enable R/W
/*
 * 7:     1 – Write TEMP_OUT_H and TEMP_OUT_L to the FIFO at the sample rate; If
 *         enabled, buffering of data occurs even if data path is in standby.
 *         0 – Function is disabled
 * 6:     1 – Write GYRO_XOUT_H and GYRO_XOUT_L to the FIFO at the sample rate; If
 *         enabled, buffering of data occurs even if data path is in standby.
 *         0 – Function is disabled
 * 5:   1 – Write GYRO_YOUT_H and GYRO_YOUT_L to the FIFO at the sample rate; If
 *         enabled, buffering of data occurs even if data path is in standby.
 *         0 – Function is disabled
 * 4:     1 – Write GYRO_ZOUT_H and GYRO_ZOUT_L to the FIFO at the sample rate; If
 *         enabled, buffering of data occurs even if data path is in standby.
 *         0 – function is disabled
 * 3:     1 – Write ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L,
 *         ACCEL_ZOUT_H, and ACCEL_ZOUT_L to the FIFO at the sample rate;
 *         0 – Function is disabled
 * 2-0: RESERVED
 */
// 7
#define FIFO_EN_TEMP_EN BIT_7
// 6
#define FIFO_EN_XG_FIFO_EN BIT_6
// 5
#define FIFO_EN_YG_FIFO_EN BIT_5
// 4
#define FIFO_EN_ZG_FIFO_EN BIT_4
// 3
#define FIFO_EN_ACCEL_FIFO_EN BIT_3

//-------------------------------------------
// FSYNC INTERRUPT STATUS R to Clear
/*
 * This bit automatically sets to 1 when a FSYNC interrupt has been generated.
 * The bit clears to 0 after the register has been read.
 */

//-------------------------------------------
// – INT/DRDY PIN / BYPASS ENABLE CONFIGURATION R/W
/*
 * 7: 1 – The logic level for INT/DRDY pin is active low.
 *       0 – The logic level for INT/DRDY pin is active high.
 * 6: 1 – INT/DRDY pin is configured as open drain.
 *       0 – INT/DRDY pin is configured as push-pull.
 * 5: 1 – INT/DRDY pin level held until interrupt status is cleared.
 *       0 – INT/DRDY pin indicates interrupt pulse’s width is 50us.
 * 4: 1 – Interrupt status is cleared if any read operation is performed.
 *       0 – Interrupt status is cleared only by reading INT_STATUS register
 * 3: 1 – The logic level for the FSYNC pin as an interrupt is active low.
 *       0 – The logic level for the FSYNC pin as an interrupt is active high.
 * 2: 1 – The FSYNC pin will trigger an interrupt when it transitionsto the level
 *       specified by FSYNC_INT_LEVEL.
 *       0 – The FSYNC pin is disabled from causing an interrupt.
 * 1-0: Reserved
 */
// 7
#define INT_PIN_CFG_LEVEL_ACT_LOW BIT_7
// 6
#define INT_PIN_CFG_OPEN_DRAIN BIT_6
// 5
#define INT_PIN_CFG_LATCH_EN  BIT_5
// 4
#define INT_PIN_CFG_LEVEL_RD_CLEAR_TRUE BIT_4
// 3
#define INT_PIN_CFG_FSYNC_LEVEL_ACT_LOW  BIT_3 // REMEMBER FSYNC IS HELD LOW BY DESIGN!!
// 2
#define INT_PIN_CFG_FSYNC_INT_MODE_EN BIT_2

//-------------------------------------------
// INTERRUPT ENABLE R/W
/*
 * 7-5: 111 – Enable WoM interrupt on accelerometer.
 *         000 – Disable WoM interrupt on accelerometer.
 * 4: 1 – Enables a FIFO buffer overflow to generate an interrupt.
 *       0 – Function is disabled.
 * 3: RESERVED
 * 2: Gyroscope Drive System Ready interrupt enable
 * 1: DMP interrupt enable
 * 0: Data ready interrupt enable
 */
// 7-5
#define INT_ENABLE_WOM_EN 0xE0
// 4
#define INT_ENABLE_FIFO_OFLOW_EN BIT_4
// 2
#define INT_ENABLE_GDRIVE_EN  BIT_2
// 1
#define INT_ENABLE_DMP_EN BIT_1
// 0
#define INT_ENABLE_DATA_RDY_EN BIT_0

//-------------------------------------------
// DMP INTERRUPT STATUS R to clear
/*
 * 7-6: RESERVED
 * 5-0: DMP Interrupts
 */

//-------------------------------------------
// INTERRUPT STATUS R to clear
/*
 * 7-5: Accelerometer WoM interrupt status. Cleared on Read.
 *         111 – WoM interrupt on accelerometer
 *
 * 4:This bit automatically sets to 1 when a FIFO buffer overflow has been
 *      generated. The bit clears to 0 after the register has been read.
 * 3: RESERVED
 * 2: Gyroscope Drive System Ready interrupt
 * 1: DMP interrupt
 * 0: This bit automatically sets to 1 when a Data Ready interrupt is generated. The
 *    bit clears to 0 after the register has been read.
 *
 */

//-------------------------------------------
// ACCEL Measurements R

//-------------------------------------------
// TEMP Measurements R

//-------------------------------------------
// GYRO Measurements R

//-------------------------------------------
// SIGNAL PATH RESET R/W
/*
 * 7-2: Reserved
 * 1: Reset accel digital signal path. Note: Sensor registers are not cleared. Use
 *       SIG_COND_RST to clear sensor registers.
 * 2: Reset temp digital signal path. Note: Sensor registers are not cleared. Use
 *       SIG_COND_RST to clear sensor registers.
 */
// 1
#define SIGNAL_PATH_RESET_ACCEL_RST BIT_1
// 0
#define SIGNAL_PATH_RESET_TEMP_RST BIT_0

//-------------------------------------------
// ACCELEROMETER INTELLIGENCE CONTROL R/W
/*
 * 7: This bit enables the Wake-on-Motion detection logic
 * 6: 0 – Do not use
 *       1 – Compare the current sample with the previous sample
 * 5-0: RESERVED
 */
// 7
#define ACCEL_INTEL_CTRL_ACCEL_INTEL_EN BIT_7
// 6
#define ACCEL_INTEL_CTRL_ACCEL_INTEL_MODE_ON BIT_6

//-------------------------------------------
// USER CONTROL R/W
/*
 * 7: Enable DMP.
 * 6: 1 – Enable FIFO operation mode.
 *       0 – Disable FIFO access from serial interface. To disable FIFO writes by DMA, use
 *       FIFO_EN register. To disable possible FIFO writes from DMP, disable the DMP.
 * 5: RESERVED
 * 4: 1 – Disable I2C Slave module and put the serial interface in SPI mode only.
 * 3: Reset DMP.
 * 2: 1 – Reset FIFO module. Reset is asynchronous. This bit auto clears after one clock
 *       cycle of the internal 20MHz clock.
 * 1: RESERVED
 * 0: 1 – Reset all gyro digital signal path, accel digital signal path, and temp digital signal
 *       path. This bit also clears all the sensor registers.
 */
// 7
#define USER_CTRL_DMP_EN BIT_7
// 6
#define USER_CTRL_FIFO_EN BIT_6
// 4
#define USER_CTRL_I2C_IF_DIS BIT_4
// 3
#define USER_CTRL_DMP_RST BIT_3
// 2
#define USER_CTRL_FIFO_RST BIT_2
// 0
#define USER_CTRL_SIG_COND_RST BIT_0

//-------------------------------------------
// POWER MANAGEMENT1 R/W
/*
 * 7: 1 – Reset the internal registers and restores the default settings. The bit
 *       automatically clears to 0 once the reset is done.
 * 6: 1 – The chip is set to sleep mode.
 *       Note: The default value is 1; the chip comes up in Sleep mode
 * 5: When set to 1, and SLEEP and STANDBY are not set to 1, the chip will cycle between
 *    sleep and taking a single accelerometer sample at a rate determined by
 *       SMPLRT_DIV
 * 4: When set, the gyro drive and pll circuitry are enabled, but the sense paths are
 *       disabled. This is a low power mode that allows quick enabling of the gyros.
 * 3: When set to 1, this bit disables the temperature sensor.
 * 2-0: Clock Select
 */
// 7
#define PWR_MGMT_1_DEVICE_RESET BIT_7
// 6
#define PWR_MGMT_1_SLEEP BIT_6
// 5
#define PWR_MGMT_1_ACCEL_CYCLE BIT_5
// 4
#define PWR_MGMT_1_GYRO_STANDBY BIT_4
// 3
#define PWR_MGMT_1_TEMP_DIS BIT_3
// 2-0
#define PWR_MGMT_1_CLKSEL_20MHZ 0x00
#define PWR_MGMT_1_CLKSEL_AUTO BIT_0
#define PWR_MGMT_1_CLKSEL_OFF 0x07

//-------------------------------------------
// POWER MANAGEMENT2 R/W
/*
 * 7: 1 – Enable FIFO in low-power accelerometer mode. Default setting is 0.
 * 6: 1 - Disable DMP execution in low-power accelerometer mode. Default setting is 0.
 * 5: 1 – X accelerometer is disabled
 *       0 – X accelerometer is on
 * 4: 1 – Y accelerometer is disabled
 *       0 – Y accelerometer is on
 * 3: 1 – Z accelerometer is disabled
 *       0 – Z accelerometer is on
 * 2: 1 – X gyro is disabled
 *       0 – X gyro is on
 * 1: 1 – Y gyro is disabled
 *       0 – Y gyro is on
 * 0: 1 – Z gyro is disabled
 *       0 – Z gyro is on
 */
// 7
#define PWR_MGMT_2_FIFO_LP_EN BIT_7
// 6
#define PWR_MGMT_2_DMP_LP_DIS BIT_6
// 5
#define PWR_MGMT_2_STBY_XA BIT_5
// 4
#define PWR_MGMT_2_STBY_YA BIT_4
// 3
#define PWR_MGMT_2_STBY_ZA BIT_3
// 2
#define PWR_MGMT_2_STBY_XG BIT_2
// 1
#define PWR_MGMT_2_STBY_YG BIT_1
// 0
#define PWR_MGMT_2_STBY_ZG BIT_0

//-------------------------------------------
// FIFO COUNT REGISTERS R
/*
 * 7-5: RESERVED
 * 4-0: High Bits [12:8], count indicates the number of written bytes in the FIFO.
 *         Reading this byte latches the data for both FIFO_COUNTH, and FIFO_COUNTL.
 */
/*
 * 7-0: Low Bits [7:0].
 */

//-------------------------------------------
// FIFO READ WRITE R/W
/*
 *    7-0: Read/Write command provides Read or Write operation for the FIFO.
 */

//-------------------------------------------
// WHO AM I R
/*
 * 7-0: Register to indicate to user which device is being accessed.
 */

//-------------------------------------------
// ACCEL OFFSET REG R/W
/*
 *    Refer to data-sheet page 50
 */

//-----------------------------------------------------------------------------
// Chosen scales
//-----------------------------------------------------------------------------

#define ACCEL_SENSITIVITY_CHOSEN    ACCEL_SENSITIVTY_4G
#define ACCEL_CONFIG1_FS_SEL_CHOSEN    ACCEL_CONFIG1_FS_SEL_4G

#define ROOM_TEMP_OFFSET 0 // raw integer, zero corresponds to 25 deg C

#define GYRO_SENSITIVITY_CHOSEN        GYRO_SENSITIVITY_500DPS
#define GYRO_CONFIG_FS_SEL_CHOSEN    GYRO_CONFIG_FS_SEL_500DPS

//-----------------------------------------------------------------------------
// Function List
//-----------------------------------------------------------------------------

/*
 * @brief        begins the IMU for simple polling.
 * @note
 * @param        none
 * @return        none
 */
void NU_IMU_Init();

/*
 * @brief        writes a byte to a register.
 * @note        uses polling, should only be used for beginning.
 * @param        the register's address,
 * @param        the byte to be sent,
 * @return        none
 */
void NU_IMU_WriteReg(uint8_t addr, uint8_t data);

/*
 * @brief        reads a byte from a register.
 * @note        uses polling, should only be used for testing and debugging.
 * @param        the register's address,
 * @param        a pointer to the byte to be read,
 * @return        none
 */
void NU_IMU_ReadReg(uint8_t addr, uint8_t* data);

/*
 * @brief        reads multiple consecutive registers in a burst.
 * @note        Use this as a temporary replacement of the FIFO.
 *                 This uses DMA.
 * @param        the address of the first register to be read,
 * @param        an array of the bytes to be read,
 * @param        the length, i.e. the number of registers to be read,
 * @return        none
 */
void NU_IMU_ReadBurst(uint8_t addrs, uint8_t* data, uint16_t length);

/*
 * @brief        reads multiple registers in turns as a temporary solution.
 * @note        Yes, it is slow and inefficient, but it works. This is intended as a naive back-up.
 * @param        an array of the registers' addresses in order to be read,
 * @param        an array of the bytes to be read, the first of which is padding,
 * @param        the length, i.e. the number of registers to be read,
 * @return        none
 */
void NU_IMU_ReadSlowly(uint8_t* addrs, uint8_t* data, uint16_t length);

/*
 * @brief        reads the fifo.
 * @note        Does not work yet.
 * @param        an array of the bytes to be read, the first of which is padding,
 * @param        the length, i.e. the number of registers to be read,
 * @return        none
 */
void NU_IMU_ReadFifo(uint8_t* data, uint16_t length);

/*
 * @brief        converts raw integers into floating decimals.
 * @note        accelerometer values are in g's, and gyroscope values are in dps.
 * @param        the raw data to be converted from,
 * @param        the converted data,
 * @return        none
 */
void NU_IMU_ConvertRawData(struct NU_IMU_raw_data* raw_data, struct NU_IMU_converted_data* converted_data);

/*
 * @brief        checks for the interrupt-flags for the receiving to be done.
 * @param        none,
 * @retval        #true if the receiving was done, i.e. something has been received,
 * @retval        #false if nothing has not been received yet,
 */
bool NU_IMU_CheckForReceive();

/*
 * @brief        checks for the interrupt-flags for the transmitting to be done.
 * @param        none,
 * @retval        #true if the transmitting was done,
 * @retval        #false if not everything has been transmitted yet,
 */
bool NU_IMU_CheckForTransmit();

/*
 * @brief        used for blocking to get the next byte from the IMU.
 * @note        may not be needed strictly.
 * @param        the data to be sent,
 * @param        the number of bytes,
 * @return        none
 */
void NU_IMU_TransmitReceive_IT(uint8_t* tx_data, uint8_t* rx_data, uint16_t length);

/*
 * @brief        used for blocking to get the next byte from the IMU.
 * @note        may not be needed strictly.
 * @param        the data
 * @return        none
 */
void NU_IMU_BlockingTransmit(uint8_t* data, uint16_t length);

//Legacy functions (fixed):

void NU_IMU_Transmit(uint8_t adr, uint8_t dat, int byte_size);

void NU_IMU_TransmitReceive(uint8_t adr, uint8_t dat, uint8_t* dat_return, int byte_size);

//-----------------------------------------------------------------------------
// External variables
//-----------------------------------------------------------------------------

extern uint8_t SPI_flag;
extern uint8_t INT_flag;

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

struct NU_IMU_raw_data {
    uint8_t pad;
    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    } accelerometer;
    int16_t temperature;
    struct {
        int16_t x;
        int16_t y;
        int16_t z;
    } gyroscope;
};

struct NU_IMU_converted_data {
    uint8_t ID;
    struct {
        float x;
        float y;
        float z;
    } accelerometer;
    float temperature;
    struct {
        float x;
        float y;
        float z;
    } gyroscope;
};

#endif //_IMU_H_


