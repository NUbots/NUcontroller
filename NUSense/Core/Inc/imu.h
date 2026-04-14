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
 * Ported from NUfsr to NUSense for testing.
 *
 ***********************************************************/

/**
 * IMU class usage instructions:
 *
 * 1. Create instance:
 *  nusense::IMU imu{};
 *
 * 2. Create structs for IMU data (you may not need raw_data)
 *  nusense::IMU::RawData raw_data;
 *  nusense::IMU::ConvertedData converted_data;
 *
 * 3. Start the imu:
 *  imu.init();
 *
 * 4. The IMU can be read in four ways:
 *  a) Easy and most useful way:
 *      // read *new* data and return in converted form
 *      converted_data = imu.get_new_converted_data();
 *
 *  c) Getting new data entirely manually:
 *      // get *new* IMU data on each call
 *      imu.get_new_raw_data();
 *      // convert the data within the class
 *      imu.generate_converted_data();
 *      // get the converted data (*not* a new read)
 *      converted_data = imu.get_last_converted_data();
 *
 *  d) Getting existing old raw data if you want that for some reason?
 *      raw_data = imu.get_last_raw_data();
 *
 */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _IMU_H_
#define _IMU_H_

/* Includes */
#include <stdint.h>

#include "main.h"
#include "settings.h"
#include "spi.h"


namespace nusense {

    //---------------Constants----------------//
    // Masks for the flags for each SPI interface, in this case only one:
    const uint8_t SPI4_RX = 0x40U;
    const uint8_t SPI4_TX = 0x80U;

    //                          /* 7 6 5 4 3 2 1 0 */
    const uint8_t BIT_0 = 0x01; /* 0 0 0 0 0 0 0 1 -> 0x01 */
    const uint8_t BIT_1 = 0x02; /* 0 0 0 0 0 0 1 0 -> 0x02 */
    const uint8_t BIT_2 = 0x04; /* 0 0 0 0 0 1 0 0 -> 0x04 */
    const uint8_t BIT_3 = 0x08; /* 0 0 0 0 1 0 0 0 -> 0x08 */
    const uint8_t BIT_4 = 0x10; /* 0 0 0 1 0 0 0 0 -> 0x10 */
    const uint8_t BIT_5 = 0x20; /* 0 0 1 0 0 0 0 0 -> 0x20 */
    const uint8_t BIT_6 = 0x40; /* 0 1 0 0 0 0 0 0 -> 0x40 */
    const uint8_t BIT_7 = 0x80; /* 1 0 0 0 0 0 0 0 -> 0x80 */
    const uint8_t BLANK = 0x00;

    // These thresholds determine when the IMU must be repolled because of erroneous data. Each
    // threshold is by how much difference is between the current step-difference and the previous
    // step-difference. These values were determined as roughly half the observed spikes. Tuning
    // these values down may make the spikes less frequent but may mean that the IMU is polled
    // needlessly; tuning these values up will poll the IMU less often but may make the spikes more
    // frequent.
    constexpr float ACCELEROMETER_SPIKE_THRESHOLD = 8;
    constexpr float GYROSCOPE_SPIKE_THRESHOLD     = 4;

    class IMU {
    public:
        /// @brief Constructs and initialises the IMU
        /// @todo  Add configuration parameters in constructor
        IMU() {
            // Initialize the IMU
            init();
        };

        virtual ~IMU(){};

        // From Table-1, Table-2, and Table-3 from the datasheet.
        const uint16_t ACCEL_SENSITIVTY_2G     = 16384;  // LSB/g
        const uint16_t ACCEL_SENSITIVTY_4G     = 8192;   // LSB/g
        const uint16_t ACCEL_SENSITIVTY_8G     = 4096;   // LSB/g
        const uint16_t ACCEL_SENSITIVTY_16G    = 2048;   // LSB/g
        const float TEMP_SENSITIVITY           = 326.8;  // LSB/degC
        const uint16_t GYRO_SENSITIVITY_250DPS = 131;    // LSB/(deg/s)
        const float GYRO_SENSITIVITY_500DPS    = 65.5;   // LSB/(deg/s)
        const float GYRO_SENSITIVITY_1000DPS   = 32.8;   // LSB/(deg/s)
        const float GYRO_SENSITIVITY_2000DPS   = 16.4;   // LSB/(deg/s)

        //---------------Address Byte Defines----------------//
        /* Common Defines */
        const uint8_t IMU_READ  = 0x80;
        const uint8_t IMU_WRITE = 0x00;
        const uint8_t IMU_DEVICE_ID = 0x98;
        enum class Address : uint8_t {
            /* Command Defines */ /* ADR Register Function */
            // Self Test
            SELF_TEST_X_GYRO  = 0x00, /* Gyro X Self test Reg */
            SELF_TEST_Y_GYRO  = 0x01, /* Gyro Y Self test Reg */
            SELF_TEST_Z_GYRO  = 0x02, /* Gyro Z Self test Reg */
            SELF_TEST_X_ACCEL = 0x0D, /* Accel X Self test Reg */
            SELF_TEST_Y_ACCEL = 0x0E, /* Accel Y Self test Reg */
            SELF_TEST_Z_ACCEL = 0x0F, /* Accel Z Self test Reg */

            // Offset adjustment
            XG_OFFS_USRH = 0x13, /* Gyro X Offset Adjustment Reg 15:8 */
            XG_OFFS_USRL = 0x14, /* Gyro X Offset Adjustment Reg 7:0 */
            YG_OFFS_USRH = 0x15, /* Gyro Y Offset Adjustment Reg 15:8 */
            YG_OFFS_USRL = 0x16, /* Gyro Y Offset Adjustment Reg 7:0 */
            ZG_OFFS_USRH = 0x17, /* Gyro Z Offset Adjustment Reg 15:8 */
            ZG_OFFS_USRL = 0x18, /* Gyro Z Offset Adjustment Reg 7:0 */

            // Configuration
            SMPLRT_DIV    = 0x19, /* Sample Rate divider */
            CONFIG        = 0x1A, /* Configuration */
            GYRO_CONFIG   = 0x1B, /* Gyro Configuration */
            ACCEL_CONFIG  = 0x1C, /* Accel Configuration */
            ACCEL_CONFIG2 = 0x1D, /* Accel Configuration */
            LP_MODE_CFG   = 0x1E, /* Low Power Mode Config */

            // Interrupt Configuration
            ACCEL_WOM_X_THR = 0x20, /* X-Axis Accel Wake-On Motion Threshold */
            ACCEL_WOM_Y_THR = 0x21, /* Y-Axis Accel Wake-On Motion Threshold */
            ACCEL_WOM_Z_THR = 0x22, /* Z-Axis Accel Wake-On Motion Threshold */
            FIFO_EN         = 0x23, /* FIFO Enable */
            FSYNC_INT       = 0x36, /* FSYNC Interrupt Status */
            INT_PIN_CFG     = 0x37, /* INT/DRDY PIN / BYPASS Enable Config */
            INT_ENABLE      = 0x38, /* Interrupt Enable */
            DMP_INT_STATUS  = 0x39, /* DMP Interrupt Status */
            INT_STATUS      = 0x3A, /* Interrupt Status */

            // Accelerometer Measurements
            ACCEL_XOUT_H = 0x3B, /* X_Accel High byte */
            ACCEL_XOUT_L = 0x3C, /* X_Accel Low byte */
            ACCEL_YOUT_H = 0x3D, /* Y_Accel High byte */
            ACCEL_YOUT_L = 0x3E, /* Y_Accel Low byte */
            ACCEL_ZOUT_H = 0x3F, /* Z_Accel High byte */
            ACCEL_ZOUT_L = 0x40, /* Z_Accel Low byte */

            // Temperature Measurements
            TEMP_OUT_H = 0x41, /* Temp High Byte */
            TEMP_OUT_L = 0x42, /* Temp Low Byte */

            // Gyroscope Measurements
            GYRO_XOUT_H = 0x43, /* X_GYRO High byte */
            GYRO_XOUT_L = 0x44, /* X_GYRO Low byte */
            GYRO_YOUT_H = 0x45, /* Y_GYRO High byte */
            GYRO_YOUT_L = 0x46, /* Y_GYRO Low byte */
            GYRO_ZOUT_H = 0x47, /* Z_GYRO High byte */
            GYRO_ZOUT_L = 0x48, /* Z_GYRO Low byte */

            // Additional Options/Configurations
            SIGNAL_PATH_RESET = 0x68, /* Signal Path Reset */
            ACCEL_INTEL_CTRL  = 0x69, /* Accel Intelligence Control */
            USER_CTRL         = 0x6A, /* User Control */

            // Power Management
            PWR_MGMT_1 = 0x6B, /* Power Management 1 */
            PWR_MGMT_2 = 0x6C, /* Power Management 2 */

            // FIFO Commands
            FIFO_COUNTH = 0x72, /* High FIFO count byte */
            FIFO_COUNTL = 0x73, /* LOW FIFO count byte */
            FIFO_R_W    = 0x74, /* FIFO buffer Read/Write 8 */

            // WHO AM I
            WHO_AM_I = 0x75, /* Device Information Register */

            // ACCEL Offset Reg
            XA_OFFSET_H = 0x77, /* X_Accel High byte offset calculation */
            XA_OFFSET_L = 0x78, /* X_Accel Low byte offset calculation */
            YA_OFFSET_H = 0x7A, /* Y_Accel High byte offset calculation */
            YA_OFFSET_L = 0x7B, /* Y_Accel Low byte offset calculation */
            ZA_OFFSET_H = 0x7D, /* Z_Accel High byte offset calculation */
            ZA_OFFSET_L = 0x7E, /* Z_Accel Low byte offset calculation */
        };

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
         * 7:   Always set to 0
         * 6:   When set to ‘1’, when the FIFO is full, additional writes will not be written to FIFO.
         *      When set to ‘0’, when the FIFO is full, additional writes will be written to the FIFO,
         *      replacing the oldest data.
         * 5-3: Enables the FSYNC pin data to be sampled (Pin is held at 0 by NUfsr design).
         * 2-0: Digital Low Pas Filter Config: Refer to data sheet page 39
         */
        // 6:
        const uint8_t CONFIG_FIFO_MODE_OVERFLOW_WAIT = BIT_6;

        //-------------------------------------------
        // GYRO Configuration R/W
        /*
         * 7-5: Gyro self-test
         * 4-3: Gyro Full Scale select
         * 2:   RESERVED
         * 1-0: FCHOICE_B
         */
        // 7-5
        const uint8_t GYRO_CONFIG_X_SELF_TEST = BIT_7;
        const uint8_t GYRO_CONFIG_Y_SELF_TEST = BIT_6;
        const uint8_t GYRO_CONFIG_Z_SELF_TEST = BIT_5;
        // 4-3
        const uint8_t GYRO_CONFIG_FS_SEL_250DPS  = BLANK;
        const uint8_t GYRO_CONFIG_FS_SEL_500DPS  = BIT_3;
        const uint8_t GYRO_CONFIG_FS_SEL_1000DPS = BIT_4;
        const uint8_t GYRO_CONFIG_FS_SEL_2000DPS = BIT_3 | BIT_4;
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
        const uint8_t ACCEL_CONFIG1_X_SELF_TEST = BIT_7;
        const uint8_t ACCEL_CONFIG1_Y_SELF_TEST = BIT_6;
        const uint8_t ACCEL_CONFIG1_Z_SELF_TEST = BIT_5;
        // 4-3
        const uint8_t ACCEL_CONFIG1_FS_SEL_2G  = BLANK;
        const uint8_t ACCEL_CONFIG1_FS_SEL_4G  = BIT_3;
        const uint8_t ACCEL_CONFIG1_FS_SEL_8G  = BIT_4;
        const uint8_t ACCEL_CONFIG1_FS_SEL_16G = BIT_3 | BIT_4;

        //-------------------------------------------
        // ACCEL Configuration2 R/W
        /*
         * 7-6: FIFO Size
         * 5-4: Averaging filter settings for Low Power Accelerometer mode:
         * 3:   FCHOICE_B: Used to bypass DLPF.
         * 2-0: Accelerometer low pass filter setting.
         */
        // 7-6
        const uint8_t ACCEL_CONFIG2_FIFO_SIZE_512B = BLANK;
        const uint8_t ACCEL_CONFIG2_FIFO_SIZE_1KB  = BIT_6;
        const uint8_t ACCEL_CONFIG2_FIFO_SIZE_2KB  = BIT_7;
        const uint8_t ACCEL_CONFIG2_FIFO_SIZE_4KB  = BIT_6 | BIT_7;
        // 5-4
        const uint8_t ACCEL_CONFIG2_DEC2_CFG_4SAMPLES  = BLANK;
        const uint8_t ACCEL_CONFIG2_DEC2_CFG_8SAMPLES  = BIT_4;
        const uint8_t ACCEL_CONFIG2_DEC2_CFG_16SAMPLES = BIT_5;
        const uint8_t ACCEL_CONFIG2_DEC2_CFG_32SAMPLES = BIT_4 | BIT_5;
        // 3
        const uint8_t ACCEL_CONFIG2_ACCEL_FCHOICE_B_TRUE  = BIT_3;
        const uint8_t ACCEL_CONFIG2_ACCEL_FCHOICE_B_FALSE = BLANK;
        // 2-0
        // Refer to data-sheet page 41

        //-------------------------------------------
        // Low power mode config R/W
        /*
         * 7:   When set to ‘1’ low-power gyroscope mode is enabled. Default
         *      setting is ‘0’
         * 6-4: Averaging filter configuration for low-power gyroscope mode.
         *      Default setting is ‘000’. Refer to data-sheet page 42
         * 3-0: RESERVED
         */
        // 7
        const uint8_t LP_MODE_CFG_GYRO_CYCLE_TRUE = BIT_7;

        //-------------------------------------------
        // Wake on motion threshold R/W
        /*
         * 7-0: Wake on Motion Interrupt threshold for respective axis accelerometer.
         */

        //-------------------------------------------
        // FIFO Enable R/W
        /*
         * 7:   1 – Write TEMP_OUT_H and TEMP_OUT_L to the FIFO at the sample rate; If
         *          enabled, buffering of data occurs even if data path is in standby.
         *      0 – Function is disabled
         * 6:   1 – Write GYRO_XOUT_H and GYRO_XOUT_L to the FIFO at the sample rate; If
         *          enabled, buffering of data occurs even if data path is in standby.
         *      0 – Function is disabled
         * 5:   1 – Write GYRO_YOUT_H and GYRO_YOUT_L to the FIFO at the sample rate; If
         *          enabled, buffering of data occurs even if data path is in standby.
         *      0 – Function is disabled
         * 4:   1 – Write GYRO_ZOUT_H and GYRO_ZOUT_L to the FIFO at the sample rate; If
         *          enabled, buffering of data occurs even if data path is in standby.
         *      0 – function is disabled
         * 3:   1 – Write ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L,
         *          ACCEL_ZOUT_H, and ACCEL_ZOUT_L to the FIFO at the sample rate;
         *      0 – Function is disabled
         * 2-0: RESERVED
         */
        // 7
        const uint8_t FIFO_EN_TEMP_EN = BIT_7;
        // 6
        const uint8_t FIFO_EN_XG_FIFO_EN = BIT_6;
        // 5
        const uint8_t FIFO_EN_YG_FIFO_EN = BIT_5;
        // 4
        const uint8_t FIFO_EN_ZG_FIFO_EN = BIT_4;
        // 3
        const uint8_t FIFO_EN_ACCEL_FIFO_EN = BIT_3;

        //-------------------------------------------
        // FSYNC INTERRUPT STATUS R to Clear
        /*
         * This bit automatically sets to 1 when a FSYNC interrupt has been generated.
         * The bit clears to 0 after the register has been read.
         */

        //-------------------------------------------
        // – INT/DRDY PIN / BYPASS ENABLE CONFIGURATION R/W
        /*
         * 7:   1 – The logic level for INT/DRDY pin is active low.
         *      0 – The logic level for INT/DRDY pin is active high.
         * 6:   1 – INT/DRDY pin is configured as open drain.
         *      0 – INT/DRDY pin is configured as push-pull.
         * 5:   1 – INT/DRDY pin level held until interrupt status is cleared.
         *      0 – INT/DRDY pin indicates interrupt pulse’s width is 50us.
         * 4:   1 – Interrupt status is cleared if any read operation is performed.
         *      0 – Interrupt status is cleared only by reading INT_STATUS register
         * 3:   1 – The logic level for the FSYNC pin as an interrupt is active low.
         *      0 – The logic level for the FSYNC pin as an interrupt is active high.
         * 2:   1 – The FSYNC pin will trigger an interrupt when it transitionsto the level
         *          specified by FSYNC_INT_LEVEL.
         *      0 – The FSYNC pin is disabled from causing an interrupt.
         * 1-0: Reserved
         */
        // 7
        const uint8_t INT_PIN_CFG_LEVEL_ACT_LOW = BIT_7;
        // 6
        const uint8_t INT_PIN_CFG_OPEN_DRAIN = BIT_6;
        // 5
        const uint8_t INT_PIN_CFG_LATCH_EN = BIT_5;
        // 4
        const uint8_t INT_PIN_CFG_LEVEL_RD_CLEAR_TRUE = BIT_4;
        // 3
        const uint8_t INT_PIN_CFG_FSYNC_LEVEL_ACT_LOW = BIT_3;  // REMEMBER FSYNC IS HELD LOW BY DESIGN!!
        // 2
        const uint8_t INT_PIN_CFG_FSYNC_INT_MODE_EN = BIT_2;

        //-------------------------------------------
        // INTERRUPT ENABLE R/W
        /*
         * 7-5: 111 – Enable WoM interrupt on accelerometer.
         *         000 – Disable WoM interrupt on accelerometer.
         * 4:   1 – Enables a FIFO buffer overflow to generate an interrupt.
         *      0 – Function is disabled.
         * 3:   RESERVED
         * 2:   Gyroscope Drive System Ready interrupt enable
         * 1:   DMP interrupt enable
         * 0:   Data ready interrupt enable
         */
        // 7-5
        const uint8_t INT_ENABLE_WOM_EN = BIT_5 | BIT_6 | BIT_7;
        // 4
        const uint8_t INT_ENABLE_FIFO_OFLOW_EN = BIT_4;
        // 2
        const uint8_t INT_ENABLE_GDRIVE_EN = BIT_2;
        // 1
        const uint8_t INT_ENABLE_DMP_EN = BIT_1;
        // 0
        const uint8_t INT_ENABLE_DATA_RDY_EN = BIT_0;

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
         *      111 – WoM interrupt on accelerometer
         *
         * 4:   This bit automatically sets to 1 when a FIFO buffer overflow has been
         *      generated. The bit clears to 0 after the register has been read.
         * 3:   RESERVED
         * 2:   Gyroscope Drive System Ready interrupt
         * 1:   DMP interrupt
         * 0:   This bit automatically sets to 1 when a Data Ready interrupt is generated. The
         *      bit clears to 0 after the register has been read.
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
         * 1:   Reset accel digital signal path. Note: Sensor registers are not cleared. Use
         *      SIG_COND_RST to clear sensor registers.
         * 2:   Reset temp digital signal path. Note: Sensor registers are not cleared. Use
         *      SIG_COND_RST to clear sensor registers.
         */
        // 1
        const uint8_t SIGNAL_PATH_RESET_ACCEL_RST = BIT_1;
        // 0
        const uint8_t SIGNAL_PATH_RESET_TEMP_RST = BIT_0;

        //-------------------------------------------
        // ACCELEROMETER INTELLIGENCE CONTROL R/W
        /*
         * 7:   This bit enables the Wake-on-Motion detection logic
         * 6:   0 – Do not use
         *      1 – Compare the current sample with the previous sample
         * 5-0: RESERVED
         */
        // 7
        const uint8_t ACCEL_INTEL_CTRL_ACCEL_INTEL_EN = BIT_7;
        // 6
        const uint8_t ACCEL_INTEL_CTRL_ACCEL_INTEL_MODE_ON = BIT_6;

        //-------------------------------------------
        // USER CONTROL R/W
        /*
         * 7: Enable DMP.
         * 6: 1 – Enable FIFO operation mode.
         *    0 – Disable FIFO access from serial interface. To disable FIFO writes by DMA, use
         *        FIFO_EN register. To disable possible FIFO writes from DMP, disable the DMP.
         * 5: RESERVED
         * 4: 1 – Disable I2C Slave module and put the serial interface in SPI mode only.
         * 3: Reset DMP.
         * 2: 1 – Reset FIFO module. Reset is asynchronous. This bit auto clears after one clock
         *        cycle of the internal 20MHz clock.
         * 1: RESERVED
         * 0: 1 – Reset all gyro digital signal path, accel digital signal path, and temp digital signal
         *        path. This bit also clears all the sensor registers.
         */
        // 7
        const uint8_t USER_CTRL_DMP_EN = BIT_7;
        // 6
        const uint8_t USER_CTRL_FIFO_EN = BIT_6;
        // 4
        const uint8_t USER_CTRL_I2C_IF_DIS = BIT_4;
        // 3
        const uint8_t USER_CTRL_DMP_RST = BIT_3;
        // 2
        const uint8_t USER_CTRL_FIFO_RST = BIT_2;
        // 0
        const uint8_t USER_CTRL_SIG_COND_RST = BIT_0;

        //-------------------------------------------
        // POWER MANAGEMENT1 R/W
        /*
         * 7:   1 – Reset the internal registers and restores the default settings. The bit
         *          automatically clears to 0 once the reset is done.
         * 6:   1 – The chip is set to sleep mode.
         *          Note: The default value is 1; the chip comes up in Sleep mode
         * 5:   When set to 1, and SLEEP and STANDBY are not set to 1, the chip will cycle between
         *      sleep and taking a single accelerometer sample at a rate determined by
         *      SMPLRT_DIV
         * 4:   When set, the gyro drive and pll circuitry are enabled, but the sense paths are
         *      disabled. This is a low power mode that allows quick enabling of the gyros.
         * 3:   When set to 1, this bit disables the temperature sensor.
         * 2-0: Clock Select
         */
        // 7
        const uint8_t PWR_MGMT_1_DEVICE_RESET = BIT_7;
        // 6
        const uint8_t PWR_MGMT_1_SLEEP = BIT_6;
        // 5
        const uint8_t PWR_MGMT_1_ACCEL_CYCLE = BIT_5;
        // 4
        const uint8_t PWR_MGMT_1_GYRO_STANDBY = BIT_4;
        // 3
        const uint8_t PWR_MGMT_1_TEMP_DIS = BIT_3;
        // 2-0
        const uint8_t PWR_MGMT_1_CLKSEL_20MHZ = BLANK;
        const uint8_t PWR_MGMT_1_CLKSEL_AUTO  = BIT_0;
        const uint8_t PWR_MGMT_1_CLKSEL_OFF   = 0x07;

        //-------------------------------------------
        // POWER MANAGEMENT2 R/W
        /*
         * 7: 1 – Enable FIFO in low-power accelerometer mode. Default setting is 0.
         * 6: 1 - Disable DMP execution in low-power accelerometer mode. Default setting is 0.
         * 5: 1 – X accelerometer is disabled
         *    0 – X accelerometer is on
         * 4: 1 – Y accelerometer is disabled
         *    0 – Y accelerometer is on
         * 3: 1 – Z accelerometer is disabled
         *    0 – Z accelerometer is on
         * 2: 1 – X gyro is disabled
         *    0 – X gyro is on
         * 1: 1 – Y gyro is disabled
         *    0 – Y gyro is on
         * 0: 1 – Z gyro is disabled
         *    0 – Z gyro is on
         */
        // 7
        const uint8_t PWR_MGMT_2_FIFO_LP_EN = BIT_7;
        // 6
        const uint8_t PWR_MGMT_2_DMP_LP_DIS = BIT_6;
        // 5
        const uint8_t PWR_MGMT_2_STBY_XA = BIT_5;
        // 4
        const uint8_t PWR_MGMT_2_STBY_YA = BIT_4;
        // 3
        const uint8_t PWR_MGMT_2_STBY_ZA = BIT_3;
        // 2
        const uint8_t PWR_MGMT_2_STBY_XG = BIT_2;
        // 1
        const uint8_t PWR_MGMT_2_STBY_YG = BIT_1;
        // 0
        const uint8_t PWR_MGMT_2_STBY_ZG = BIT_0;

        //-------------------------------------------
        // FIFO COUNT REGISTERS
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
        // WHO AM I
        /*
         * 7-0: Register to indicate to user which device is being accessed.
         */

        //-------------------------------------------
        // ACCEL OFFSET REG R/W
        /*
         *    Refer to data-sheet page 50
         */

        //-----------------------------------------------------------------------------
        // Configs
        //-----------------------------------------------------------------------------

        const uint16_t ACCEL_SENSITIVITY_CHOSEN   = ACCEL_SENSITIVTY_4G;
        const uint8_t ACCEL_CONFIG1_FS_SEL_CHOSEN = ACCEL_CONFIG1_FS_SEL_4G;

        const uint8_t ROOM_TEMP_OFFSET = 0;  // raw integer, zero corresponds to 25 deg C

        const float GYRO_SENSITIVITY_CHOSEN     = GYRO_SENSITIVITY_500DPS;
        const uint8_t GYRO_CONFIG_FS_SEL_CHOSEN = GYRO_CONFIG_FS_SEL_500DPS;


        // make it easily to centrally change config if we ever need to
        const Address READ_BLOCK_START = Address::ACCEL_XOUT_H;
        const uint8_t READ_BLOCK_LEN   = 14;

        //-----------------------------------------------------------------------------
        // Structures
        //-----------------------------------------------------------------------------
        struct big_endian_u16 {
            uint8_t h;
            uint8_t l;
        };
        struct RawData {
            struct {
                big_endian_u16 x;
                big_endian_u16 y;
                big_endian_u16 z;
            } accelerometer;
            big_endian_u16 temperature;
            struct {
                big_endian_u16 x;
                big_endian_u16 y;
                big_endian_u16 z;
            } gyroscope;
        } __attribute__((packed));

        struct CombinedData {
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

        struct ConvertedData {
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

        //-----------------------------------------------------------------------------
        // Function List
        //-----------------------------------------------------------------------------

        /*
         * @brief   begins the IMU for simple polling.
         * @note
         * @param   none
         * @return  none
         */
        void init();

        /*
         * @brief   performs post-startup tests (POSTs)
         * @note    reads WHOAMI 1-byte register, asserts equal to 0x98
         * @param   none
         * @return  true if everything ok, false if not
         */
        bool post();

        /*
         * @brief   writes a byte to a register.
         * @note    uses polling, should only be used for beginning.
         * @param   the register's address,
         * @param   the byte to be sent,
         * @return  none
         */
        void write_reg(Address addr, uint8_t data);

        /*
         * @brief   reads a byte from a register.
         * @note    uses polling, should only be used for testing and debugging.
         * @param   the register's address,
         * @param   a pointer to the byte to be read,
         * @return  none
         */
        void read_reg(Address addr, uint8_t* data);

        /*
         * @brief   reads multiple consecutive registers in a burst.
         * @note    Use this as a temporary replacement of the FIFO.
         * @param   the address of the first register to be read,
         * @param   an array of the bytes to be read,
         * @param   the length, i.e. the number of registers to be read,
         * @return  none
         */
        void read_burst(Address addr, uint8_t* data, uint16_t length);

        /*
         * @brief   converts raw integers into floating decimals.
         * @note    accelerometer values are in g's, and gyroscope values are in dps.
         * @param   the raw data to be converted from,
         * @param   the converted data,
         * @return  none
         */
        void convert_raw_data(IMU::RawData* raw_data, IMU::ConvertedData* converted_data);

        /*
         * @brief   fill converted data based on raw data
         * @return  none
         */
        void generate_converted_data(void);

        /*
         * @brief   get new data, return it, donezo
         */
        RawData get_new_raw_data(void);

        /*
         * @brief   get new data, convert it, return it, donezo
         */
        ConvertedData get_new_converted_data(void);

        /*
         * @brief   a simple getter for the current stored raw data
         */
        RawData get_last_raw_data(void);

        /*
         * @brief   a simple getter for the current stored converted data
         */
        ConvertedData get_last_converted_data(void);

    protected:
    private:
        // structs to hold internal state of imu to read easily
        RawData raw_data;
        ConvertedData converted_data;
        ConvertedData difference;
    };

    //-----------------------------------------------------------------------------
    // External variables
    //-----------------------------------------------------------------------------

    extern uint8_t SPI_flag;
    extern uint8_t INT_flag;

}  // namespace nusense

#endif  //_IMU_H_
