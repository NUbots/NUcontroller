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
#include "usb/PacketHandler.hpp"
#include "usbd_cdc_if.h"

#include "utility/message/hash.hpp"

volatile uint8_t hash_true       = 0;
uint8_t buf[8]                   = {0};
const auto hash                  = utility::message::NUSENSE_HASH;
volatile uint64_t servo_hash     = utility::message::SERVO_TARGETS_HASH;
volatile uint64_t got_servo_hash = 0;

volatile uint8_t is_correct_hash = 0;
volatile uint64_t nuc_ts         = 0;

usb::PacketHandler packet_handler{};
message_actuation_ServoTargets targets;

namespace test_hw {

    uint64_t read_le_64(const uint8_t* ptr) {
        return (uint64_t(ptr[0]) << 0) | (uint64_t(ptr[1]) << 8) | (uint64_t(ptr[2]) << 16) | (uint64_t(ptr[3]) << 24)
               | (uint64_t(ptr[4]) << 32) | (uint64_t(ptr[5]) << 40) | (uint64_t(ptr[6]) << 48)
               | (uint64_t(ptr[7]) << 56);
    }

    void test_streamreactor() {
        NUSense::IMU imu{};
        NUSense::IMU::ConvertedData converted_data;
        message_platform_NUSense nusense_msg;
        imu.init();

        while (1) {
            // Send data over
            converted_data = imu.getNewConvertedData();

            // IMU data
            nusense_msg.imu.has_accel = true;
            nusense_msg.imu.accel.x   = converted_data.accelerometer.x;
            nusense_msg.imu.accel.y   = converted_data.accelerometer.y;
            nusense_msg.imu.accel.z   = converted_data.accelerometer.z;

            nusense_msg.imu.has_gyro = true;
            nusense_msg.imu.gyro.x   = converted_data.gyroscope.x;
            nusense_msg.imu.gyro.y   = converted_data.gyroscope.y;
            nusense_msg.imu.gyro.z   = converted_data.gyroscope.z;

            nusense_msg.imu.temperature = converted_data.temperature;
            nusense_msg.has_imu         = true;

            // Fill servo entries (fake)
            nusense_msg.servo_map_count = 20;
            for (size_t i = 0; i < 20; ++i) {
                nusense_msg.servo_map[i].key       = i;
                nusense_msg.servo_map[i].has_value = true;

                nusense_msg.servo_map[i].value.id             = i + 1;
                nusense_msg.servo_map[i].value.hardware_error = 1;
                nusense_msg.servo_map[i].value.torque_enabled = true;

                nusense_msg.servo_map[i].value.present_pwm      = 3.0;
                nusense_msg.servo_map[i].value.present_current  = 4.0;
                nusense_msg.servo_map[i].value.present_velocity = 5.0;
                nusense_msg.servo_map[i].value.present_position = 6.0;

                nusense_msg.servo_map[i].value.goal_pwm      = 7.0;
                nusense_msg.servo_map[i].value.goal_current  = 8.0;
                nusense_msg.servo_map[i].value.goal_velocity = 9.0;
                nusense_msg.servo_map[i].value.goal_position = 10.0;

                nusense_msg.servo_map[i].value.voltage     = 11.0;
                nusense_msg.servo_map[i].value.temperature = 12.0;
            }

            // Allow max size for the output buffer so it doesn't throw an error if there's not enough space
            uint8_t payload[2048];
            pb_ostream_t output_buffer = pb_ostream_from_buffer(&payload[0], 1600);

            if (pb_encode(&output_buffer, message_platform_NUSense_fields, &nusense_msg)) {
                // Happiness. Parse nbs packet
                std::vector<uint8_t> nbs({0xE2, 0x98, 0xA2});

                // TODO fill this up properly
                uint64_t ts_u = 0;
                uint32_t size = uint32_t(output_buffer.bytes_written + sizeof(hash) + sizeof(ts_u));

                for (size_t i = 0; i < sizeof(size); ++i) {
                    nbs.push_back(uint8_t((size >> (i * 8)) & 0xFF));
                }

                // Timestamp
                for (size_t i = 0; i < sizeof(ts_u); ++i) {
                    nbs.push_back(uint8_t((ts_u >> (i * 8)) & 0xFF));
                }

                // Hash
                for (size_t i = 0; i < sizeof(hash); ++i) {
                    nbs.push_back(uint8_t((hash >> (i * 8)) & 0xFF));
                }

                nbs.insert(nbs.end(), std::begin(payload), std::begin(payload) + output_buffer.bytes_written);

                CDC_Transmit_HS(nbs.data(), nbs.size());
                HAL_Delay(10);
            }

            // Receive data from the other side
            if (packet_handler.handle_incoming()) {
                targets = *packet_handler.get_targets();

                is_correct_hash = utility::message::SERVO_TARGETS_HASH == packet_handler.get_curr_msg_hash() ? 1 : 0;
                nuc_ts          = packet_handler.get_curr_msg_timestamp();
                got_servo_hash  = packet_handler.get_curr_msg_hash();
            }
        }
    }
}  // namespace test_hw
#endif /* SRC_TEST_HW_HPP_ */
