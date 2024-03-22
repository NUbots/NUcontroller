#include "../NUsenseIO.hpp"

namespace platform::NUsense {
    bool NUsenseIO::send_servo_states() {
        NU_IMU_ReadBurst(ACCEL_XOUT_H, IMU_rx, 14);


        raw_data.accelerometer.x = ((uint16_t) IMU_rx[0] << 8) | IMU_rx[1];
        raw_data.accelerometer.y = ((uint16_t) IMU_rx[2] << 8) | IMU_rx[3];
        raw_data.accelerometer.z = ((uint16_t) IMU_rx[4] << 8) | IMU_rx[5];
        raw_data.temperature     = ((uint16_t) IMU_rx[6] << 8) | IMU_rx[7];
        raw_data.gyroscope.x     = ((uint16_t) IMU_rx[8] << 8) | IMU_rx[9];
        raw_data.gyroscope.y     = ((uint16_t) IMU_rx[10] << 8) | IMU_rx[11];
        raw_data.gyroscope.z     = ((uint16_t) IMU_rx[12] << 8) | IMU_rx[13];

        NU_IMU_ConvertRawData(&raw_data, &converted_data);

        // TODO: (JohanneMontano) Handle IMU read and conversions if it fails
        // Fill the struct with the values we converted from the IMU output
        // For now we just say we have these values regardless if the read and conversion fails or not
        // We have to do it this way because nanopb will not encode the submessage field if has_msg is set to false
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

        // Fill servo entries using the data in servo_states
        nusense_msg.servo_map_count = NUMBER_OF_DEVICES;

        for (size_t i = 0; i < NUMBER_OF_DEVICES; ++i) {
            nusense_msg.servo_map[i].key       = i;
            nusense_msg.servo_map[i].has_value = true;

            nusense_msg.servo_map[i].value.id             = i + 1;
            nusense_msg.servo_map[i].value.hardware_error = servo_states[i].hardware_error;
            nusense_msg.servo_map[i].value.torque_enabled = servo_states[i].torque_enabled;

            nusense_msg.servo_map[i].value.present_pwm      = servo_states[i].present_pwm;
            nusense_msg.servo_map[i].value.present_current  = servo_states[i].present_current;
            nusense_msg.servo_map[i].value.present_velocity = servo_states[i].present_velocity;
            nusense_msg.servo_map[i].value.present_position = servo_states[i].present_position;

            nusense_msg.servo_map[i].value.goal_pwm      = servo_states[i].goal_pwm;
            nusense_msg.servo_map[i].value.goal_current  = servo_states[i].goal_current;
            nusense_msg.servo_map[i].value.goal_velocity = servo_states[i].goal_velocity;
            nusense_msg.servo_map[i].value.goal_position = servo_states[i].goal_position;

            nusense_msg.servo_map[i].value.voltage     = servo_states[i].voltage;
            nusense_msg.servo_map[i].value.temperature = servo_states[i].temperature;
        }

        // Once everything else is filled we send it to the NUC. Just overwrite the bytes within encoding_payload
        // Allow max size for the output buffer so it doesn't throw an error if there's not enough space
        // If one wishes to add messages to the protobuf message, one must first calculate the maximum bytes
        // within that message and then add enough bytes to make sure that nanopb doesn't cry about the output stream
        // being too small If the MAX_ENCODE_SIZE is inadequately defined, one can get a corrupted message and nanopb
        // errors.
        pb_ostream_t output_buffer = pb_ostream_from_buffer(&encoding_payload[0], MAX_ENCODE_SIZE);

        if (!pb_encode(&output_buffer, message_platform_NUSense_fields, &nusense_msg)) {
            // Encoding failed, we won't bother to send a packet
            nanopb_encoding_err = true;

            return false;
        }

        // Happiness, the encoding succeeded
        // TODO: Add extra fields so we can actually parse an authentic nbs packet
        std::vector<uint8_t> full_msg({0xE2, 0x98, 0xA2});

        // Get the length of the encoded protobuf message
        int payload_length = output_buffer.bytes_written;

        std::vector<uint8_t> length_vector{static_cast<uint8_t>((payload_length >> 8) & 0xFF),
                                           static_cast<uint8_t>(payload_length & 0xFF)};

        full_msg.insert(full_msg.end(), length_vector.begin(), length_vector.end());
        full_msg.insert(full_msg.end(), std::begin(encoding_payload), std::begin(encoding_payload) + payload_length);


        if (CDC_Transmit_HS(full_msg.data(), full_msg.size()) != USBD_OK) {
            // Going into this block means that the usb failed to transmit our data
            usb_tx_err = true;
            return false;
        }

        return true;
    }
}  // namespace platform::NUsense