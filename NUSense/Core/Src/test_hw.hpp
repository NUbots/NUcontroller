/*
 * test_hw.hpp
 *
 *  Created on: 26 Feb. 2023
 *      Author: Clayton
 */

#ifndef SRC_TEST_HW_HPP_
#define SRC_TEST_HW_HPP_

#include "imu.h"
#include "settings.h"
#include "stm32h7xx_hal.h"
#include "usbd_cdc_if.h"

namespace test_hw {

#ifdef TEST_IMU
    void imu() {
        // Create our IMU instance
        nusense::IMU imu{};
        // structs to hold data
        nusense::IMU::RawData raw_data;
        nusense::IMU::ConvertedData converted_data;

        // variables for testing
        uint8_t rx[14];
        char str[256];
        uint8_t error = 0;

        // lets start this baby
        imu.init();

        while (1) {
            /* test read_reg */
            rx[0] = 0xFF;  // clear to known state
            imu.read_reg(nusense::IMU::Address::WHO_AM_I, rx);
            if (rx[0] != 0x98) {
                error++;
            }

            /* test read_burst with a single readval */
            rx[0] = 0xFF;  // clear to known state
            imu.read_burst(nusense::IMU::Address::WHO_AM_I, rx, 1);
            if (rx[0] != 0x98) {
                error++;
            }

            /* test read_burst with multiple readvals */
            /* NOTE this one fails and im not sure why... */
            for (int i = 0; i < 14; i++)
                rx[i] = 0xFF;                                        // clear to known state
            imu.read_burst(nusense::IMU::Address::FIFO_R_W, rx, 3);  // include reg either side of WHO AM I
            if (rx[1] != 0x98) {
                error++;
            }


            /* test just acc x vals */
            for (int i = 0; i < 14; i++)
                rx[i] = 0xFF;  // clear to known state
            imu.read_burst(nusense::IMU::Address::ACCEL_XOUT_H, rx, 2);
            uint16_t acc_x_raw = (rx[0] << 8) | rx[1];                                          // swap byte order
            float acc_x_conv   = static_cast<float>(acc_x_raw) / imu.ACCEL_SENSITIVITY_CHOSEN;  // conversion

            /* test all sensor vals */
            for (int i = 0; i < 14; i++)
                rx[i] = 0xFF;  // clear to known state
            imu.read_burst(imu.READ_BLOCK_START, rx, imu.READ_BLOCK_LEN);
            raw_data = *(reinterpret_cast<nusense::IMU::RawData*>(rx));  // cast raw bytes to struct
            imu.convert_raw_data(&raw_data, &converted_data);

            /* test class form, resets form breakpoint locations */

            // reset vals to be sure;
            raw_data       = {};
            converted_data = {};
            // check with first raw data generation
            raw_data = imu.get_new_raw_data();
            imu.generate_converted_data();
            converted_data = imu.get_last_converted_data();

            // reset again
            raw_data       = {};
            converted_data = {};
            // check with opposite of before. the vals here shouldn't align
            raw_data       = imu.get_last_raw_data();
            converted_data = imu.get_new_converted_data();

            // reset one last time
            raw_data       = {};
            converted_data = {};
            // check if read on demand works
            raw_data = imu.get_last_raw_data();  // read data
            imu.get_new_raw_data();              // get fresh data
            raw_data = imu.get_last_raw_data();  // read it again

            sprintf(str,
                    "IMU:\t"
                    "ACC (g):\t%.3f\t%.3f\t%.3f\t"
                    "TEMP (deg C):\t%.3f\t"
                    "GYR (dps):\t%.3f\t%.3f\t%.3f\t"
                    "Raw:\t%x%x\t%x%x\t%x%x\t%x%x\t%x%x\t%x%x\t%x%x\r\n",
                    converted_data.accelerometer.x,
                    converted_data.accelerometer.y,
                    converted_data.accelerometer.z,
                    converted_data.temperature,
                    converted_data.gyroscope.x,
                    converted_data.gyroscope.y,
                    converted_data.gyroscope.z,
                    raw_data.accelerometer.x.h,
                    raw_data.accelerometer.x.l,
                    raw_data.accelerometer.y.h,
                    raw_data.accelerometer.y.l,
                    raw_data.accelerometer.z.h,
                    raw_data.accelerometer.z.l,
                    raw_data.temperature.h,
                    raw_data.temperature.l,
                    raw_data.gyroscope.x.h,
                    raw_data.gyroscope.x.l,
                    raw_data.gyroscope.y.h,
                    raw_data.gyroscope.y.l,
                    raw_data.gyroscope.z.h,
                    raw_data.gyroscope.z.l);

            CDC_Transmit_HS((uint8_t*) str, strlen(str));

            HAL_Delay(100);
        }
    }
#endif

}  // namespace test_hw

#endif /* SRC_TEST_HW_HPP_ */
