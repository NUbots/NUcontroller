/*
 * read_imu.cpp
 *
 *  Created on: Mar 8, 2024
 *      Author: jmontano
 */

#include "../NUsenseIO.hpp"

namespace platform::NUsense {
    void NUsenseIO::read_imu() {
        NU_IMU_ReadBurst(ACCEL_XOUT_H, IMU_rx, 14);

        raw_data.accelerometer.x = ((uint16_t) IMU_rx[0] << 8) | IMU_rx[1];
        raw_data.accelerometer.y = ((uint16_t) IMU_rx[2] << 8) | IMU_rx[3];
        raw_data.accelerometer.z = ((uint16_t) IMU_rx[4] << 8) | IMU_rx[5];
        raw_data.temperature     = ((uint16_t) IMU_rx[6] << 8) | IMU_rx[7];
        raw_data.gyroscope.x     = ((uint16_t) IMU_rx[8] << 8) | IMU_rx[9];
        raw_data.gyroscope.y     = ((uint16_t) IMU_rx[10] << 8) | IMU_rx[11];
        raw_data.gyroscope.z     = ((uint16_t) IMU_rx[12] << 8) | IMU_rx[13];

        NU_IMU_ConvertRawData(&raw_data, &converted_data);
    }
}  // namespace platform::NUsense
