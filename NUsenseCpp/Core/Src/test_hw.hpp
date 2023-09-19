/*
 * test_hw.hpp
 *
 *  Created on: 26 Feb. 2023
 *      Author: Clayton
 */

#include "stm32h7xx_hal.h"

#include "settings.h"
#include "usbd_cdc_if.h"
#include "imu.h"

#include "dynamixel/Devices.hpp"
#include "dynamixel/Packetiser.hpp"
#include "dynamixel/Packet.hpp"
#include "uart/Port.hpp"
#include "uart/rs485_c.h"
#include "uart/RS485.h"

#ifndef SRC_TEST_HW_HPP_
#define SRC_TEST_HW_HPP_

namespace test_hw {

uint8_t calculate_checksum(uint8_t* data, uint8_t length);
uint16_t update_crc(uint16_t crc_accum, uint8_t* data_blk_ptr, uint16_t data_blk_size);

#ifdef TEST_UART
void uart() {
    /* This example in interrupt-mode is not the most efficient one. It freezes
    * when it is bombarded with characters with no other delay in the main loop.
    * This bug needs further investigation.
    * Update: I think that this has been fixed now that DMA is working although
    * it is safer to keep an eye out for it.
    */
    char str_buffer[] = "NUsense = nuisance!\r\n";
    uart::RS485 link = uart::RS485(TEST_UART);
    char c = 'x';
    // Wait for the first packet.
    //link.receive((uint8_t*)&c, 1);

    while (1) {
        // Echo whatever is received on the test UART.
        // If a packet has been received, then send back the character and receive
        // the next one.
        /*if (link.get_receive_flag())
            link.transmit((uint8_t*)&c, 1);
        if (link.get_transmit_flag())
            //link.transmit((uint8_t*)&str_buffer, strlen(str_buffer));
            link.receive((uint8_t*)&c, 1);*/
        link.receive_poll((uint8_t*)&c, 1, HAL_MAX_DELAY);
        link.transmit_poll((uint8_t*)&c, 1, HAL_MAX_DELAY);
    }
}
#endif

#ifdef TEST_USB
void usb() {
    char str_buffer[] = "NUsense = nuisance!\r\n";

    while (1) {
        CDC_Transmit_HS((uint8_t*)str_buffer, strlen(str_buffer));
    }
}
#endif

#ifdef TEST_IMU
void imu() {
    int16_t acc;
    uint16_t count;
    uint8_t flags;
    uint8_t rx[14];
    struct NU_IMU_raw_data raw_data;
    struct NU_IMU_converted_data converted_data;
    char str[256];

    NU_IMU_Init();

    while (1) {
        //NU_IMU_ReadSlowly(addresses, (uint8_t*)&raw_data, 16);
        //NU_IMU_ReadFifo(rx, 14);
        //NU_IMU_ReadFifo(rx, 4);
        //NU_IMU_ReadBurst(ACCEL_XOUT_H, rx, 14);

        NU_IMU_ReadBurst(ACCEL_XOUT_H, rx, 14);

        raw_data.accelerometer.x = ((uint16_t)rx[ 0] << 8) | rx[ 1];
        raw_data.accelerometer.y = ((uint16_t)rx[ 2] << 8) | rx[ 3];
        raw_data.accelerometer.z = ((uint16_t)rx[ 4] << 8) | rx[ 5];
        raw_data.temperature =        ((uint16_t)rx[ 6] << 8) | rx[ 7];
        raw_data.gyroscope.x =     ((uint16_t)rx[ 8] << 8) | rx[ 9];
        raw_data.gyroscope.y =     ((uint16_t)rx[10] << 8) | rx[11];
        raw_data.gyroscope.z =     ((uint16_t)rx[12] << 8) | rx[13];

        NU_IMU_ConvertRawData(&raw_data, &converted_data);

        sprintf(str, "IMU:\t"
                "ACC (g):\t%.3f\t%.3f\t%.3f\t"
                "TEMP (deg C):\t%.3f\t"
                "GYR (dps):\t%.3f\t%.3f\t%.3f\t"
                "Raw:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\r\n",
                converted_data.accelerometer.x,
                converted_data.accelerometer.y,
                converted_data.accelerometer.z,
                converted_data.temperature,
                converted_data.gyroscope.x,
                converted_data.gyroscope.y,
                converted_data.gyroscope.z,
                raw_data.accelerometer.x,
                raw_data.accelerometer.y,
                raw_data.accelerometer.z,
                raw_data.temperature,
                raw_data.gyroscope.x,
                raw_data.gyroscope.y,
                raw_data.gyroscope.z
                );

        CDC_Transmit_HS((uint8_t*)str, strlen(str));

        HAL_Delay(100);
    }
}
#endif

#ifdef TEST_PORT
void port() {
    char str_buffer[64];
    uint16_t byte = 0xAA55;
    uart::Port port(1);
    port.begin_rx();

    while (1) {
        byte = port.read();
        if (byte != NO_BYTE_READ) {
            sprintf(str_buffer, "NUsense = nuisance! %c\r\n", (uint8_t)byte);
            port.write((uint8_t*)str_buffer, strlen(str_buffer));
        }
        //test_port.write((uint8_t*)str_buffer, strlen(str_buffer));
        // Always check both interrupts at the end of the context in which one is
        // using this class.
        //port.check_rx();
        //port.check_tx();
        //HAL_Delay(1);
    }
}
#endif

#ifdef TEST_MOTOR
void motor_v1() {
    uart::RS485 rs_link(1);
    uint8_t inst_packet[] = {
          0xFF, 0xFF,     // header
          0x01,            // ID
          0x04,            // packet's length
          0x03,            // instruction
          0x19,            // address of data
          0x01,            // data to be written
          0xFF            // checksum
    };
    //inst_packet[7] = calculate_checksum(&inst_packet[2], 5);
    uint8_t sts_packet[6+3];
    rs_link.transmit(inst_packet, 8);

    while (1) {
        // Version 1.0
        if (rs_link.get_transmit_flag())
            rs_link.receive(sts_packet, 6);

        if (rs_link.get_receive_flag()) {
            inst_packet[6] ^= 0x01;
            inst_packet[7] = calculate_checksum(&inst_packet[2], 5);
            HAL_Delay(500);
            rs_link.transmit(inst_packet, 8);
        }
    }
}

void motor_v2() {
    /* Please mind that a lot of this code is very bare-bones. We will need a
    * proper protocol-handler later.
    */
    /*
    * Most of this code for the Packetiser class and the Packet struct was
    * hastily taken from the original NUClear code. It has been bastardised
    * here. It may very well need a redesign. For example, I am not sure how
    * useful the Packet struct is since the Packetiser needs vectors since the
    * byte-stuffing makes a packet's length unpredictable (at least in unlikely
    * in situations when byte-stuffing is actually needed).
    */
    uart::Port port(1);
    dynamixel::Packetiser packetiser;
    std::array<uint8_t,4> params = {0xA8,0x00,0x41,0x00};
    std::array<uint8_t,3> params_1 = {0xE0,0x00,0x01};
    std::vector<uint8_t> inst_packet(sizeof(dynamixel::Packet<uint8_t, 4>));

    // Encode the instruction-packet to write to an indirect address.
    new (inst_packet.data())
          dynamixel::Packet<uint8_t, 4>(0x01, dynamixel::WRITE, params);
    packetiser.encode(inst_packet);

    /* Delay for a bit to give the motor time to boot up. Without this delay, I
    * found that the motor does not respond at all. From some basic testing I
    * think that it is because the motor only boots up until the DXL power is
    * switched on from the DXL_POWER_EN pin. However, it may have something to
    * do with the RS485 transceivers instead.
    */
    HAL_Delay(1000);

    // Begin the receiving. This should be done only once if we are using the DMA
    // as a buffer.
    port.begin_rx();

    // One should always flush before expecting a packet. It is not needed right
    // after the begin_rx(), but it is here for the sake of an example.
    port.flush_rx();

    // Write the isntruction-packet to the port.
    port.write(inst_packet.data(), inst_packet.size());
    // Wait until we have received at least the expected number of bytes of the
    // status-packet. It may be longer because of byte-stuffing.
    while (port.get_available_rx() < sizeof(dynamixel::Packet<uint8_t,1>))
      // While waiting, check the TX interrupts in case the buffer is full, etc.
      port.check_tx();

    /*
    * What happens if we received the need number of bytes, but something went
    * wrong, and the packet is incomplete in the buffer? Mostly likely the code
    * will get stuck in the do-while loop below. It should never happen in
    * theory, but a safeguard may be to use some kind of timeout. A timeout is
    * needed above anyway in case we never get a packet at all.
    */

    // Decode whatever is received on the port.
    //while (!packetiser.decode(port.read()));
    do {
      // If there is a byte to be read, then decode it.
      if (port.peek() != NO_BYTE_READ)
          packetiser.decode(port.read());
    } while (!packetiser.is_packet_ready());
    // Cast to a packet-structure.
    dynamixel::Packet<uint8_t,1> sts_packet
      = *reinterpret_cast<dynamixel::Packet<uint8_t,1>*>(
          packetiser.get_decoded_packet()
      );
    // Maybe check the CRC here if this is a proper protocol-handler.
    // Reset the decoder.
    packetiser.reset();

    // Re-fit the instruction-packet to write to the indirect register.
    inst_packet.resize(sizeof(dynamixel::Packet<uint8_t, 3>));
    new (inst_packet.data())
          dynamixel::Packet<uint8_t, 3>(0x01, dynamixel::WRITE, params_1);
    // Then encode it and write it to the port.
    packetiser.encode(inst_packet);
    port.write(inst_packet.data(), inst_packet.size());

    while (1) {
        // Version 2.0

        // Wait until we have received at least the expected number of bytes of the
        // status-packet. It may be longer because of byte-stuffing.
        if (port.get_available_rx() >= sizeof(dynamixel::Packet<uint8_t,1>)) {
            // Decode whatever is received on the port.
            //while (!packetiser.decode(port.read()));
            do {
                if (port.peek() != NO_BYTE_READ)
                    packetiser.decode(port.read());
            } while (packetiser.is_packet_ready());
            // Cast to a packet-structure.
            sts_packet = *reinterpret_cast<dynamixel::Packet<uint8_t,1>*>(
                      packetiser.get_decoded_packet()
            );
            // Maybe check the CRC here if this is a proper protocol-handler.
            // Reset the decoder.
            packetiser.reset();

            // Delay so that the LED can be observed to blink at 2 Hz.
            HAL_Delay(500);

            // Toggle the state of the LED.
            params_1[2] ^= 0x01;
            new (inst_packet.data())
                    dynamixel::Packet<uint8_t, 3>(0x01, dynamixel::WRITE, params_1);
            // Then encode it and write it to the port.
            packetiser.encode(inst_packet);
            port.flush_rx();
            port.write(inst_packet.data(), inst_packet.size());
        }

        // Always check the TX interrupts.
        port.check_tx();
    }
}

uint8_t calculate_checksum(uint8_t* data, uint8_t length) {
    uint8_t checksum = 0;
    for (int i = 0; i < length; i++)
        checksum += data[i];
    return ~checksum;
}

uint16_t update_crc(uint16_t crc_accum, uint8_t* data_blk_ptr, uint16_t data_blk_size)
{
    uint16_t i, j;
    uint16_t crc_table[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
    };

    for(j = 0; j < data_blk_size; j++)
    {
        i = ((uint16_t)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
    }

    return crc_accum;
}

uint16_t update_crc(uint16_t crc_accum, const uint8_t byte) {
    uint16_t i;
    uint16_t crc_table[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
    };

    i = ((uint16_t)(crc_accum >> 8) ^ byte) & 0xFF;
    crc_accum = (crc_accum << 8) ^ crc_table[i];

    return crc_accum;
}

#endif

} // namespace test_hw

#endif /* SRC_TEST_HW_HPP_ */
