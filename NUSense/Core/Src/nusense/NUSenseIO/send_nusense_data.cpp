#include <string.h>

#include "../NUSenseIO.hpp"

namespace nusense {
    bool NUSenseIO::nusense_to_nuc() {
        // Get a new lot of IMU data
        IMU::ConvertedData converted_data;
        converted_data = imu.get_new_converted_data();

        // TODO: (JohanneMontano) Handle IMU read and conversions if it fails
        // Fill the struct with the values we converted from the IMU output
        // For now we just say we have these values regardless if the read and conversion fails or not
        // We have to do it this way because nanopb will not encode the submessage field if has_msg is set to false
        // Invert the axes since the PCB is upside down.
        nusense_msg.imu.has_accel = true;
        nusense_msg.imu.accel.x   = -converted_data.accelerometer.z;
        nusense_msg.imu.accel.y   = -converted_data.accelerometer.y;
        nusense_msg.imu.accel.z   = -converted_data.accelerometer.x;

        // Invert the axes since the PCB is upside down.
        nusense_msg.imu.has_gyro = true;
        nusense_msg.imu.gyro.x   = -converted_data.gyroscope.z;
        nusense_msg.imu.gyro.y   = -converted_data.gyroscope.y;
        nusense_msg.imu.gyro.z   = -converted_data.gyroscope.x;

        nusense_msg.imu.temperature = converted_data.temperature;
        nusense_msg.has_imu         = true;

        // Poll the buttons and include their states.
        nusense_msg.buttons.left   = mode_button.filter();
        nusense_msg.buttons.middle = start_button.filter();

        if (nusense_msg.buttons.left) {
            tx_led.pulse(1, false, device::Pulser::LOW);
        }

        if (nusense_msg.buttons.middle) {
            rx_led.pulse(1, false, device::Pulser::LOW);
        }

        nusense_msg.has_buttons = true;

        // Fill servo entries using the data in servo_states
        nusense_msg.servo_map_count = NUMBER_OF_DEVICES;

        for (size_t i = 0; i < NUMBER_OF_DEVICES; ++i) {
            nusense_msg.servo_map[i].key       = i;
            nusense_msg.servo_map[i].has_value = true;

            nusense_msg.servo_map[i].value.id = i + 1;

            // If no new data have been accumulated in the filter, then keep the existing values in the message.
            if (servo_states[i].filter_count != 0.0) {
                nusense_msg.servo_map[i].value.hardware_error = servo_states[i].hardware_error;
                nusense_msg.servo_map[i].value.torque_enabled = servo_states[i].torque_enabled;

                nusense_msg.servo_map[i].value.present_pwm = servo_states[i].present_pwm / servo_states[i].filter_count;
                nusense_msg.servo_map[i].value.present_current =
                    servo_states[i].present_current / servo_states[i].filter_count;
                nusense_msg.servo_map[i].value.present_velocity =
                    servo_states[i].present_velocity / servo_states[i].filter_count;
                nusense_msg.servo_map[i].value.present_position = servo_states[i].mean_present_position.get_mean();

                nusense_msg.servo_map[i].value.voltage     = servo_states[i].voltage / servo_states[i].filter_count;
                nusense_msg.servo_map[i].value.temperature = servo_states[i].temperature / servo_states[i].filter_count;
            }

            // If any of these are filtered in later revisions of the code, then move them under the above if-condition.
            nusense_msg.servo_map[i].value.goal_pwm      = servo_states[i].goal_pwm;
            nusense_msg.servo_map[i].value.goal_current  = servo_states[i].goal_current;
            nusense_msg.servo_map[i].value.goal_velocity = servo_states[i].goal_velocity;
            nusense_msg.servo_map[i].value.goal_position = servo_states[i].goal_position;

            nusense_msg.servo_map[i].value.has_packet_counts = true;
            nusense_msg.servo_map[i].value.packet_counts.total =
                servo_states[i].num_successes + servo_states[i].num_crc_errors + servo_states[i].num_packet_errors;
            nusense_msg.servo_map[i].value.packet_counts.timeouts      = servo_states[i].num_timeouts;
            nusense_msg.servo_map[i].value.packet_counts.crc_errors    = servo_states[i].num_crc_errors;
            nusense_msg.servo_map[i].value.packet_counts.packet_errors = servo_states[i].num_packet_errors;
        }

        // Once everything else is filled we send it to the NUC. Just overwrite the bytes within encoding_payload
        // Allow max size for the output buffer so it doesn't throw an error if there's not enough space
        // If one wishes to add messages to the protobuf message, one must first calculate the maximum bytes
        // within that message and then add enough bytes to make sure that nanopb doesn't cry about the output stream
        // being too small If the MAX_ENCODE_SIZE is inadequately defined, one can get a corrupted message and nanopb
        // errors.
        pb_ostream_t output_buffer = pb_ostream_from_buffer(&encoding_payload[0], MAX_ENCODE_SIZE);

        // TODO (NUSense people) Handle encoding errors properly using this member somehow
        nanopb_encoding_err = pb_encode(&output_buffer, message_platform_NUSense_fields, &nusense_msg) ? false : true;
        if (nanopb_encoding_err) {
            return false;
        }

        // Happiness, the encoding succeeded
        std::vector<uint8_t> nbs({0xE2, 0x98, 0xA2});

        // TODO (JohanneMontano) Implement timestamp field correctly, std::chrono is behaving weird and it needs to be
        // investigated
        uint64_t ts_u = 0;
        uint32_t size = uint32_t(output_buffer.bytes_written + sizeof(utility::message::NUSENSE_HASH) + sizeof(ts_u));

        // Encode size to uint8_t's
        for (size_t i = 0; i < sizeof(size); ++i) {
            nbs.push_back(uint8_t((size >> (i * 8)) & 0xFF));
        }

        // Encode timestamp
        for (size_t i = 0; i < sizeof(ts_u); ++i) {
            nbs.push_back(uint8_t((ts_u >> (i * 8)) & 0xFF));
        }

        // Encode nusense hash
        for (size_t i = 0; i < sizeof(utility::message::NUSENSE_HASH); ++i) {
            nbs.push_back(uint8_t((utility::message::NUSENSE_HASH >> (i * 8)) & 0xFF));
        }

        // Add the protobuf bytes into the nbs vector
        nbs.insert(nbs.end(), std::begin(encoding_payload), std::begin(encoding_payload) + output_buffer.bytes_written);

        // Attempt to transmit data then handle it accordingly if it fails
        if (CDC_Transmit_HS(nbs.data(), nbs.size()) != USBD_OK) {
            // Going into this block means that the usb failed to transmit our data
            usb_tx_err = true;
            return false;
        }

        return true;
    }
}  // namespace nusense
