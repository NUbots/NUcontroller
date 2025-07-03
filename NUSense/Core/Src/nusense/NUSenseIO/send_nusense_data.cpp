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
            buzzer.pulse(1, false, device::Pulser::LOW);
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

        return encode_and_transmit_nbs(nusense_msg, utility::message::NUSENSE_HASH, message_platform_NUSense_fields);
    }
}  // namespace nusense
