/*
 * encode.cpp
 *
 *  Created on: Mar 8, 2024
 *      Author: jmontano
 */

#include <vector>

#include "../NUsenseIO.hpp"
#include "usbd_cdc_if.h"

namespace platform::NUsense {

    bool NUsenseIO::encode(const std::array<platform::ServoState, NUMBER_OF_DEVICES>& local_cache,
                const struct NU_IMU_converted_data& converted_data,
                uint8_t* tx_data) {
        // IMU data
        message_platform_NUSense nusense_msg = message_platform_NUSense_init_zero;

        nusense_msg.imu.has_accel = true;
        nusense_msg.imu.accel.x = converted_data.accelerometer.x;
        nusense_msg.imu.accel.y = converted_data.accelerometer.y;
        nusense_msg.imu.accel.z = converted_data.accelerometer.z;

        nusense_msg.imu.has_gyro = true;
        nusense_msg.imu.gyro.x = converted_data.gyroscope.x;
        nusense_msg.imu.gyro.y = converted_data.gyroscope.y;
        nusense_msg.imu.gyro.z = converted_data.gyroscope.z;

        nusense_msg.imu.temperature = converted_data.temperature;
        nusense_msg.has_imu = true;

        // Fill servo entries
        nusense_msg.servo_map_count = 20;
        for (size_t i = 0; i < local_cache.size(); ++i) {
        	nusense_msg.servo_map[i].key = i;
        	nusense_msg.servo_map[i].has_value = true;

        	nusense_msg.servo_map[i].value.id = i + 1;
        	nusense_msg.servo_map[i].value.hardware_error = local_cache[i].hardware_error;
        	nusense_msg.servo_map[i].value.torque_enabled = local_cache[i].torque_enabled;

        	nusense_msg.servo_map[i].value.present_pwm = local_cache[i].present_pwm;
			nusense_msg.servo_map[i].value.present_current = local_cache[i].present_current;
			nusense_msg.servo_map[i].value.present_velocity = local_cache[i].present_velocity;
			nusense_msg.servo_map[i].value.present_position = local_cache[i].present_position;

			nusense_msg.servo_map[i].value.goal_pwm = local_cache[i].goal_pwm;
			nusense_msg.servo_map[i].value.goal_current = local_cache[i].goal_current;
			nusense_msg.servo_map[i].value.goal_velocity = local_cache[i].goal_velocity;
			nusense_msg.servo_map[i].value.goal_position = local_cache[i].goal_position;

			nusense_msg.servo_map[i].value.voltage = local_cache[i].voltage;
			nusense_msg.servo_map[i].value.temperature = local_cache[i].temperature;
        }

        pb_ostream_t output_buffer = pb_ostream_from_buffer(tx_data, MAX_ENCODE_SIZE);

        bool encode_success = pb_encode(&output_buffer, message_platform_NUSense_fields, &nusense_msg);

        return encode_success;
    }
}  // namespace platform::NUsense
