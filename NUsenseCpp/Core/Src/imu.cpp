/*
 * Benjamin Young
 * NUfsr IMU source file for basic command sequences.
 *
 * Addendum:
 * Clayton Carlon
 * 9/9/2022
 * Ported from NUfsr to NUsense for testing.
 */

#include "imu.h"

using namespace NUsense;

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
    // writeReg(Address::PWR_MGMT_1,     PWR_MGMT_1_CLKSEL_AUTO);
    writeReg(Address::PWR_MGMT_1, PWR_MGMT_1_DEVICE_RESET | PWR_MGMT_1_CLKSEL_AUTO);
    HAL_Delay(1);
    writeReg(Address::PWR_MGMT_1, PWR_MGMT_1_CLKSEL_AUTO);

    // Make sure that we are in SPI-mode.
    writeReg(Address::USER_CTRL, USER_CTRL_I2C_IF_DIS | USER_CTRL_DMP_RST | USER_CTRL_FIFO_EN | USER_CTRL_SIG_COND_RST);

    // Turn on all sensors.
    writeReg(Address::PWR_MGMT_2, 0x00);

    // Set the full-scale for the gyroscope.
    writeReg(Address::GYRO_CONFIG, GYRO_CONFIG_FS_SEL_CHOSEN);

    // Set the full-scale for the accelerometer.
    writeReg(Address::ACCEL_CONFIG, ACCEL_CONFIG1_FS_SEL_CHOSEN);

    // Set the accelerometer's LPF to 218 Hz and the lowest number of samples.
    writeReg(Address::ACCEL_CONFIG2, ACCEL_CONFIG2_DEC2_CFG_4SAMPLES | ACCEL_CONFIG2_ACCEL_FCHOICE_B_FALSE | 0x00);

    // Set the gyroscope's LPF to 250 Hz.
    writeReg(Address::CONFIG, CONFIG_FIFO_MODE_OVERFLOW_WAIT);

    // Set the sample-rate to 1 kHz.
    writeReg(Address::SMPLRT_DIV, 0x00);

    // Set the offset for gyroscope's x-axis to 90/4 = 22.
    writeReg(Address::XG_OFFS_USRH, 0x00);
    writeReg(Address::XG_OFFS_USRL, 0x16);

    // Set the offset for gyroscope's y-axis to -406/4 = -101.
    writeReg(Address::YG_OFFS_USRH, 0xFF);
    writeReg(Address::YG_OFFS_USRL, 0x9B);

    // Set the offset for gyroscope's z-axis to -61/4 = -15.
    writeReg(Address::ZG_OFFS_USRH, 0xFF);
    writeReg(Address::ZG_OFFS_USRL, 0xF1);
    /*
    // Set the offset for accelerometer's x-axis to 0, for now.
    writeReg(Address::XA_OFFSET_H,     0x00);
    writeReg(Address::XA_OFFSET_L,     0x02);

    // Set the offset for accelerometer's y-axis to 0, for now.
    writeReg(Address::YA_OFFSET_H,     0x00);
    writeReg(Address::YA_OFFSET_L,     0x02);

    // Set the offset for accelerometer's z-axis to 0, for now.
    writeReg(Address::ZA_OFFSET_H,     0x00);
    writeReg(Address::ZA_OFFSET_L,     0x02);
    */

    // Write all sensors' values in the FIFO.
    writeReg(Address::FIFO_EN,   // FIFO_EN_TEMP_EN
             FIFO_EN_XG_FIFO_EN  //|
                                 // FIFO_EN_YG_FIFO_EN |
                                 // FIFO_EN_ZG_FIFO_EN |
                                 // FIFO_EN_ACCEL_FIFO_EN
    );
}

/*
 * @brief   writes a byte to a register.
 * @note    uses polling, should only be used for beginning.
 * @param   the register's address,
 * @param   the byte to be sent,
 * @return  none
 */
void IMU::writeReg(Address addr, uint8_t data) {
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
void IMU::readReg(Address addr, uint8_t* data) {
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
void IMU::readBurst(Address addr, uint8_t* data, uint16_t length) {
    uint8_t packet[length + 1];
    uint8_t rx_data[length + 1];

    for (int i = 0; i < length + 1; i++) {
        rx_data[i] = 0xAA;
        if (i == 0)
            packet[i] = static_cast<uint8_t>(addr) | IMU_READ;
        else
            packet[i] = 0x00;
    }

    HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi4, packet, rx_data, length + 1, HAL_MAX_DELAY);
    // HAL_SPI_Transmit_DMA(&hspi4, packet, length+1);
    // HAL_SPI_Receive_DMA(&hspi4, rx_data, length+1);
    // HAL_SPI_TransmitReceive_IT(&hspi4, packet, rx_data, length+1);
    HAL_GPIO_WritePin(MPU_NSS_GPIO_Port, MPU_NSS_Pin, GPIO_PIN_SET);

    for (int i = 0; i < length; i++)
        data[i] = rx_data[i + 1];
}


/*
 * @brief   converts raw integers into floating decimals.
 * @note    accelerometer values are in g's, and gyroscope values are in dps.
 * @param   the raw data to be converted from,
 * @param   the converted data,
 * @return  none
 */
void IMU::convertRawData(IMU::raw_data* raw_data, IMU::converted_data* converted_data) {

    auto& acc  = raw_data->accelerometer;
    auto& gyro = raw_data->gyroscope;
    auto& temp = raw_data->temperature;

    // convert from big to little endian and then scale
    converted_data->accelerometer.x = static_cast<float>(acc.x.l | (acc.x.h << 8)) / ACCEL_SENSITIVITY_CHOSEN;
    converted_data->accelerometer.y = static_cast<float>(acc.y.l | (acc.y.h << 8)) / ACCEL_SENSITIVITY_CHOSEN;
    converted_data->accelerometer.z = static_cast<float>(acc.z.l | (acc.z.h << 8)) / ACCEL_SENSITIVITY_CHOSEN;
    converted_data->temperature     = static_cast<float>((temp.l | (temp.h << 8)) - ROOM_TEMP_OFFSET) / TEMP_SENSITIVITY
                                  + 25.0;  // from Section-11.23 from the datasheet
    converted_data->gyroscope.x = static_cast<float>(gyro.x.l | (gyro.x.h << 8)) / GYRO_SENSITIVITY_CHOSEN;
    converted_data->gyroscope.y = static_cast<float>(gyro.y.l | (gyro.y.h << 8)) / GYRO_SENSITIVITY_CHOSEN;
    converted_data->gyroscope.z = static_cast<float>(gyro.z.l | (gyro.z.h << 8)) / GYRO_SENSITIVITY_CHOSEN;
}