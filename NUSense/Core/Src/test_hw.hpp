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
#include "utility/message/message_hash.hpp"
#include <chrono>

namespace test_hw {
#ifdef TEST_STREAMREACTOR
void stream_reactor() {
//	NU_IMU_Init();
//	        uint8_t IMU_rx[14];
//	        NU_IMU_converted_data converted_data;
//	        NU_IMU_raw_data raw_data;
//	        uint16_t MAX_ENCODE_SIZE = 1600;
//
//	        HAL_Delay(5000);
//
//	        while (1) {
//
//	            NU_IMU_ReadBurst(ACCEL_XOUT_H, IMU_rx, 14);
//
//	            raw_data.accelerometer.x = ((uint16_t) IMU_rx[0] << 8) | IMU_rx[1];
//	            raw_data.accelerometer.y = ((uint16_t) IMU_rx[2] << 8) | IMU_rx[3];
//	            raw_data.accelerometer.z = ((uint16_t) IMU_rx[4] << 8) | IMU_rx[5];
//	            raw_data.temperature     = ((uint16_t) IMU_rx[6] << 8) | IMU_rx[7];
//	            raw_data.gyroscope.x     = ((uint16_t) IMU_rx[8] << 8) | IMU_rx[9];
//	            raw_data.gyroscope.y     = ((uint16_t) IMU_rx[10] << 8) | IMU_rx[11];
//	            raw_data.gyroscope.z     = ((uint16_t) IMU_rx[12] << 8) | IMU_rx[13];
//
//	            NU_IMU_ConvertRawData(&raw_data, &converted_data);
//
//	            // IMU data
//	            nusense_msg.imu.has_accel = true;
//	            nusense_msg.imu.accel.x   = converted_data.accelerometer.x;
//	            nusense_msg.imu.accel.y   = converted_data.accelerometer.y;
//	            nusense_msg.imu.accel.z   = converted_data.accelerometer.z;
//
//	            nusense_msg.imu.has_gyro = true;
//	            nusense_msg.imu.gyro.x   = converted_data.gyroscope.x;
//	            nusense_msg.imu.gyro.y   = converted_data.gyroscope.y;
//	            nusense_msg.imu.gyro.z   = converted_data.gyroscope.z;
//
//	            nusense_msg.imu.temperature = converted_data.temperature;
//	            nusense_msg.has_imu         = true;
//
//	            // Fill servo entries (fake)
//	            nusense_msg.servo_map_count = 20;
//	            for (size_t i = 0; i < 20; ++i) {
//	                nusense_msg.servo_map[i].key       = i;
//	                nusense_msg.servo_map[i].has_value = true;
//
//	                nusense_msg.servo_map[i].value.id             = i + 1;
//	                nusense_msg.servo_map[i].value.hardware_error = 1;
//	                nusense_msg.servo_map[i].value.torque_enabled = true;
//
//	                nusense_msg.servo_map[i].value.present_pwm      = 3.0;
//	                nusense_msg.servo_map[i].value.present_current  = 4.0;
//	                nusense_msg.servo_map[i].value.present_velocity = 5.0;
//	                nusense_msg.servo_map[i].value.present_position = 6.0;
//
//	                nusense_msg.servo_map[i].value.goal_pwm      = 7.0;
//	                nusense_msg.servo_map[i].value.goal_current  = 8.0;
//	                nusense_msg.servo_map[i].value.goal_velocity = 9.0;
//	                nusense_msg.servo_map[i].value.goal_position = 10.0;
//
//	                nusense_msg.servo_map[i].value.voltage     = 11.0;
//	                nusense_msg.servo_map[i].value.temperature = 12.0;
//	            }
//
//	            uint8_t payload[2048];
//
//	            // Allow max size for the output buffer so it doesn't throw an error if there's not enough space
//	            pb_ostream_t output_buffer = pb_ostream_from_buffer(&payload[0], MAX_ENCODE_SIZE);
//
//	            if (pb_encode(&output_buffer, message_platform_NUSense_fields, &nusense_msg)) {
//	                // Happiness
//	                // Parse nbs packet
//	                std::vector<uint8_t> full_msg({0xE2, 0x98, 0xA2});
//
//	                int payload_length = output_buffer.bytes_written;
//	                std::vector<uint8_t> length_vector{static_cast<uint8_t>((payload_length >> 8) & 0xFF),
//	                                                   static_cast<uint8_t>(payload_length & 0xFF)};
//
//	                full_msg.insert(full_msg.end(), length_vector.begin(), length_vector.end());
//	                full_msg.insert(full_msg.end(), std::begin(payload), std::begin(payload) + payload_length);
//
//	                translen = full_msg.size();
//
//	                CDC_Transmit_HS(full_msg.data(), full_msg.size());
//
//	                translen = 0;
//	            }
//	            else {
//	                // Sadness
//	                // TODO Execute handle for failed encoding call
//	            }
//	        }
	const auto ts = std::chrono::steady_clock::now();
	const auto ts_u = std::chrono::duration_cast<std::chrono::microseconds>(ts).count();

	const auto hash = utility::message::NUSENSE_HASH;

}
#endif // TEST_STREAMREACTOR

#ifdef TEST_IMU
    void imu() {
        // Create our IMU instance
        NUSense::IMU imu{};
        // structs to hold data
        NUSense::IMU::RawData raw_data;
        NUSense::IMU::ConvertedData converted_data;

        // variables for testing
        uint8_t rx[14];
        char str[256];
        uint8_t error = 0;

        // lets start this baby
        imu.init();

        while (1) {
            /* test readReg */
            rx[0] = 0xFF;  // clear to known state
            imu.readReg(NUSense::IMU::Address::WHO_AM_I, rx);
            if (rx[0] != 0x98) {
                error++;
            }

            /* test readBurst with a single readval */
            rx[0] = 0xFF;  // clear to known state
            imu.readBurst(NUSense::IMU::Address::WHO_AM_I, rx, 1);
            if (rx[0] != 0x98) {
                error++;
            }

            /* test readBurst with multiple readvals */
            /* NOTE this one fails and im not sure why... */
            for (int i = 0; i < 14; i++)
                rx[i] = 0xFF;                                       // clear to known state
            imu.readBurst(NUSense::IMU::Address::FIFO_R_W, rx, 3);  // include reg either side of WHO AM I
            if (rx[1] != 0x98) {
                error++;
            }


            /* test just acc x vals */
            for (int i = 0; i < 14; i++)
                rx[i] = 0xFF;  // clear to known state
            imu.readBurst(NUSense::IMU::Address::ACCEL_XOUT_H, rx, 2);
            uint16_t acc_x_raw = (rx[0] << 8) | rx[1];                                          // swap byte order
            float acc_x_conv   = static_cast<float>(acc_x_raw) / imu.ACCEL_SENSITIVITY_CHOSEN;  // conversion

            /* test all sensor vals */
            for (int i = 0; i < 14; i++)
                rx[i] = 0xFF;  // clear to known state
            imu.readBurst(imu.READ_BLOCK_START, rx, imu.READ_BLOCK_LEN);
            raw_data = *(reinterpret_cast<NUSense::IMU::RawData*>(rx));  // cast raw bytes to struct
            imu.convertRawData(&raw_data, &converted_data);

            /* test class form, resets form breakpoint locations */

            // reset vals to be sure;
            raw_data       = {};
            converted_data = {};
            // check with first raw data generation
            raw_data = imu.getNewRawData();
            imu.generateConvertedData();
            converted_data = imu.getLastConvertedData();

            // reset again
            raw_data       = {};
            converted_data = {};
            // check with opposite of before. the vals here shouldn't align
            raw_data       = imu.getLastRawData();
            converted_data = imu.getNewConvertedData();

            // reset one last time
            raw_data       = {};
            converted_data = {};
            // check if read on demand works
            raw_data = imu.getLastRawData();  // read data
            imu.getNewRawData();              // get fresh data
            raw_data = imu.getLastRawData();  // read it again

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
