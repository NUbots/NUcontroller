/*
 * Benjamin Young
 * NUfsr IMU source file for basic command sequences.
 *
 * Addendum:
 * Clayton Carlon
 * 9/9/2022
 * Ported from NUfsr to NUSense for testing.
 */

#include "imu.h"

#include <cmath>

namespace nusense {

    static volatile uint8_t spi_int_flags;

    void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef* hspi) {
        if (hspi == &hspi4)
            spi_int_flags |= SPI4_RX;
    }

    void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef* hspi) {
        if (hspi == &hspi4)
            spi_int_flags |= SPI4_TX;
    }

    /*
     * @brief   begins the IMU for simple polling.
     * @note
     * @param   none
     * @return  none
     */
    void IMU::init() {
        // Select the first PLL as the clock and do a soft reset.
        // This may fix the problem of the power-up sequence in Section-4.19 of the IMU's datasheet.
        write_reg(Address::PWR_MGMT_1, PWR_MGMT_1_DEVICE_RESET | PWR_MGMT_1_CLKSEL_AUTO);
        HAL_Delay(1);
        // Ensure that the first PLL is chosen.
        write_reg(Address::PWR_MGMT_1, PWR_MGMT_1_CLKSEL_AUTO);
        // For some reason, a delay of at least 1 ms is needed afterwards lest the registers are not written to.
        HAL_Delay(10);

        // Make sure that we are in SPI-mode.
        write_reg(Address::USER_CTRL,
                  USER_CTRL_I2C_IF_DIS | USER_CTRL_DMP_RST | USER_CTRL_FIFO_EN | USER_CTRL_SIG_COND_RST);

        // Turn on all sensors.
        write_reg(Address::PWR_MGMT_2, 0x00);

        // Set the full-scale for the gyroscope.
        write_reg(Address::GYRO_CONFIG, GYRO_CONFIG_FS_SEL_CHOSEN);

        // Set the full-scale for the accelerometer.
        write_reg(Address::ACCEL_CONFIG, ACCEL_CONFIG1_FS_SEL_CHOSEN);

        // Set the accelerometer's LPF to 218 Hz and the lowest number of samples.
        write_reg(Address::ACCEL_CONFIG2, ACCEL_CONFIG2_DEC2_CFG_4SAMPLES | ACCEL_CONFIG2_ACCEL_FCHOICE_B_FALSE | 0x00);

        // Set the gyroscope's LPF to 250 Hz.
        write_reg(Address::CONFIG, CONFIG_FIFO_MODE_OVERFLOW_WAIT);

        // Set the sample-rate to 1 kHz.
        write_reg(Address::SMPLRT_DIV, 0x00);

        // Set the offset for gyroscope's x-axis to 90/4 = 22.
        write_reg(Address::XG_OFFS_USRH, 0x00);
        write_reg(Address::XG_OFFS_USRL, 0x16);

        // Set the offset for gyroscope's y-axis to -406/4 = -101.
        write_reg(Address::YG_OFFS_USRH, 0xFF);
        write_reg(Address::YG_OFFS_USRL, 0x9B);

        // Set the offset for gyroscope's z-axis to -61/4 = -15.
        write_reg(Address::ZG_OFFS_USRH, 0xFF);
        write_reg(Address::ZG_OFFS_USRL, 0xF1);
        /*
        // Set the offset for accelerometer's x-axis to 0, for now.
        write_reg(Address::XA_OFFSET_H,     0x00);
        write_reg(Address::XA_OFFSET_L,     0x02);

        // Set the offset for accelerometer's y-axis to 0, for now.
        write_reg(Address::YA_OFFSET_H,     0x00);
        write_reg(Address::YA_OFFSET_L,     0x02);

        // Set the offset for accelerometer's z-axis to 0, for now.
        write_reg(Address::ZA_OFFSET_H,     0x00);
        write_reg(Address::ZA_OFFSET_L,     0x02);
        */

        /*
        // We don't use the FIFO at the moment, but this will be needed in the future
        // Write all sensors' values in the FIFO.
        write_reg(Address::FIFO_EN,
                 FIFO_EN_TEMP_EN | FIFO_EN_XG_FIFO_EN | FIFO_EN_YG_FIFO_EN | FIFO_EN_ZG_FIFO_EN |
        FIFO_EN_ACCEL_FIFO_EN);
        */
    }

    /*
     * @brief   performs post-startup tests (POSTs)
     * @note    reads WHOAMI 1-byte register, asserts equal to 0x98
     * @param   none
     * @return  true if everything ok, false if not
     */
    bool IMU::post() {
        uint8_t who_am_i = 0;
        read_reg(Address::WHO_AM_I, &who_am_i);

        if (who_am_i != IMU_DEVICE_ID) {
            return false;
        }

        return true;
    }

    /*
     * @brief   writes a byte to a register.
     * @note    uses polling, should only be used for beginning.
     * @param   the register's address,
     * @param   the byte to be sent,
     * @return  none
     */
    void IMU::write_reg(Address addr, uint8_t data) {
        uint8_t packet[2] = {static_cast<uint8_t>(addr) | IMU_WRITE, data};

        HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_RESET);
        HAL_SPI_Transmit(&hspi4, packet, 2, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_SET);
    }

    /*
     * @brief   reads a byte from a register.
     * @note    uses polling, should only be used for testing and debugging.
     * @param   the register's address,
     * @param   a pointer to the byte to be read,
     * @return  none
     */
    void IMU::read_reg(Address addr, uint8_t* data) {
        uint8_t rx_data[2] = {0xFF, 0xFF};
        uint8_t packet[2]  = {static_cast<uint8_t>(addr) | IMU_READ, 0x00};

        HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_RESET);
        HAL_SPI_TransmitReceive(&hspi4, packet, rx_data, 2, HAL_MAX_DELAY);
        HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_SET);

        *data = rx_data[1];
    }

    /*
     * @brief   reads multiple consecutive registers in a burst.
     * @note    Use this as a temporary replacement of the FIFO.
     * @param   the address of the first register to be read,
     * @param   an array of the bytes to be read,
     * @param   the length, i.e. the number of registers to be read,
     * @return  none
     */
    void IMU::read_burst(Address addr, uint8_t* data, uint16_t length) {
        uint8_t packet[length + 1];
        uint8_t rx_data[length + 1];
        HAL_StatusTypeDef status;

        for (int i = 0; i < length + 1; i++) {
            rx_data[i] = 0xAA;
            if (i == 0)
                packet[i] = static_cast<uint8_t>(addr) | IMU_READ;
            else
                packet[i] = 0x00;
        }

        // Wait for at most 1 ms for data to come back.
        HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_RESET);
        status = HAL_SPI_TransmitReceive(&hspi4, packet, rx_data, length + 1, 1);
        HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_SET);

        // If there has been new data within that time, then update the data cache.
        // Else, leave it.
        if (status == HAL_OK) {
            for (int i = 0; i < length; i++)
                data[i] = rx_data[i + 1];
        }
    }


    /*
     * @brief   converts raw integers into floating decimals.
     * @note    accelerometer values are in g's, and gyroscope values are in dps.
     * @param   the raw data to be converted from,
     * @param   the converted data,
     * @return  none
     */
    void IMU::convert_raw_data(IMU::RawData* raw_data, IMU::ConvertedData* converted_data) {

        auto& acc  = raw_data->accelerometer;
        auto& gyro = raw_data->gyroscope;
        auto& temp = raw_data->temperature;

        // endianness conversion to signed int first, otherwise float conversion fails
        CombinedData combined = {.accelerometer = {.x = static_cast<int16_t>(acc.x.l | (acc.x.h << 8)),
                                                   .y = static_cast<int16_t>(acc.y.l | (acc.y.h << 8)),
                                                   .z = static_cast<int16_t>(acc.z.l | (acc.z.h << 8))},
                                 .temperature   = static_cast<int16_t>(temp.l | (temp.h << 8)),
                                 .gyroscope     = {.x = static_cast<int16_t>(gyro.x.l | (gyro.x.h << 8)),
                                                   .y = static_cast<int16_t>(gyro.y.l | (gyro.y.h << 8)),
                                                   .z = static_cast<int16_t>(gyro.z.l | (gyro.z.h << 8))}};
        // Convert the acceleration from bits, to g's, and then to ms^-2.
        converted_data->accelerometer.x = static_cast<float>(combined.accelerometer.x) / ACCEL_SENSITIVITY_CHOSEN * 9.8;
        converted_data->accelerometer.y = static_cast<float>(combined.accelerometer.y) / ACCEL_SENSITIVITY_CHOSEN * 9.8;
        converted_data->accelerometer.z = static_cast<float>(combined.accelerometer.z) / ACCEL_SENSITIVITY_CHOSEN * 9.8;
        // Convert the rotational speed from bits, to deg/s, and then to rad/s.
        converted_data->gyroscope.x = static_cast<float>(combined.gyroscope.x) / GYRO_SENSITIVITY_CHOSEN * M_PI / 180.0;
        converted_data->gyroscope.y = static_cast<float>(combined.gyroscope.y) / GYRO_SENSITIVITY_CHOSEN * M_PI / 180.0;
        converted_data->gyroscope.z = static_cast<float>(combined.gyroscope.z) / GYRO_SENSITIVITY_CHOSEN * M_PI / 180.0;
        // Convert the temperature according to the formula in the datasheet.
        converted_data->temperature = (static_cast<float>(combined.temperature) - ROOM_TEMP_OFFSET) / TEMP_SENSITIVITY
                                      + 25.0;  // from Section-11.25 from the datasheet
    }

    /*
     * @brief   fill converted data based on existing raw data
     * @return  none
     */
    void IMU::generate_converted_data(void) {
        // sneaky lil one liner with private variable permissions
        convert_raw_data(&raw_data, &converted_data);
    };

    /*
     * @brief   get new data, return it, donezo
     */
    IMU::RawData IMU::get_new_raw_data(void) {
        // create read buffer
        uint8_t buff[14];
        // read raw vals into the buffer
        read_burst(READ_BLOCK_START, buff, READ_BLOCK_LEN);
        // cast raw vals into the internal struct
        raw_data = *(reinterpret_cast<IMU::RawData*>(buff));
        // return it the new data
        return raw_data;
        // donezo
    };

    /*
     * @brief   get new data, convert it, return it, donezo
     */
    IMU::ConvertedData IMU::get_new_converted_data(void) {
        // Store old data locally.
        ConvertedData old_converted_data = converted_data;
        // Keep the previous difference.
        ConvertedData old_difference = difference;

        // get and convert new data
        get_new_raw_data();
        convert_raw_data(&raw_data, &converted_data);

        // Work out the difference in IMU readings. This is the change in data between sampling.
        difference.accelerometer.x = converted_data.accelerometer.x - old_converted_data.accelerometer.x;
        difference.accelerometer.y = converted_data.accelerometer.y - old_converted_data.accelerometer.y;
        difference.accelerometer.z = converted_data.accelerometer.z - old_converted_data.accelerometer.z;
        difference.gyroscope.x     = converted_data.gyroscope.x - old_converted_data.gyroscope.x;
        difference.gyroscope.y     = converted_data.gyroscope.y - old_converted_data.gyroscope.y;
        difference.gyroscope.z     = converted_data.gyroscope.z - old_converted_data.gyroscope.z;

        // If the difference is much larger than the previous difference, then get new data from the IMU.
        if ((std::abs(difference.accelerometer.x - old_difference.accelerometer.x) >= ACCELEROMETER_SPIKE_THRESHOLD)
            || (std::abs(difference.accelerometer.y - old_difference.accelerometer.y) >= ACCELEROMETER_SPIKE_THRESHOLD)
            || (std::abs(difference.accelerometer.z - old_difference.accelerometer.z) >= ACCELEROMETER_SPIKE_THRESHOLD)
            || (std::abs(difference.gyroscope.x - old_difference.gyroscope.x) >= GYROSCOPE_SPIKE_THRESHOLD)
            || (std::abs(difference.gyroscope.x - old_difference.gyroscope.y) >= GYROSCOPE_SPIKE_THRESHOLD)
            || (std::abs(difference.gyroscope.x - old_difference.gyroscope.z) >= GYROSCOPE_SPIKE_THRESHOLD)) {
            get_new_raw_data();
            convert_raw_data(&raw_data, &converted_data);

            // Work out the difference again.
            difference.accelerometer.x = converted_data.accelerometer.x - old_converted_data.accelerometer.x;
            difference.accelerometer.y = converted_data.accelerometer.y - old_converted_data.accelerometer.y;
            difference.accelerometer.z = converted_data.accelerometer.z - old_converted_data.accelerometer.z;
            difference.gyroscope.x     = converted_data.gyroscope.x - old_converted_data.gyroscope.x;
            difference.gyroscope.y     = converted_data.gyroscope.y - old_converted_data.gyroscope.y;
            difference.gyroscope.z     = converted_data.gyroscope.z - old_converted_data.gyroscope.z;
        }

        // donezo
        return converted_data;
    };

    /*
     * @brief   a simple getter for the current stored raw data
     */
    IMU::RawData IMU::get_last_raw_data(void) {
        return raw_data;
    };

    /*
     * @brief   a simple getter for the current stored converted data
     */
    IMU::ConvertedData IMU::get_last_converted_data(void) {
        return converted_data;
    };

}  // namespace nusense
