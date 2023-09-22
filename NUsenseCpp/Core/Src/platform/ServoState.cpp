#include "ServoState.hpp"

#include <iomanip>  // needed to make the output stream nicer

namespace platform {

    std::ostream & operator << (std::ostream& out, const ServoState& servo_state) {
        out << "Torque En. " << std::setw(1)
            << servo_state.torque_enabled << "\t";
        out << "Pk. Err. 0x" << std::setfill('0') << std::setw(4) << std::hex
            << (uint16_t)servo_state.packet_error << "\t" << std::setfill(' ');
        out << "Hw. Err. 0x" << std::setfill('0') << std::setw(4) << std::hex
            << (uint16_t)servo_state.packet_error << "\t" << std::setfill(' ');
        out << "PWM " << std::fixed << std::setw(6) << std::setprecision(2)
            << servo_state.present_pwm << "\t";
        out << "Curr. " << std::fixed << std::setw(6) << std::setprecision(2)
            << servo_state.present_current << "\t";
        out << "Vel. " << std::fixed << std::setw(8) << std::setprecision(2)
            << servo_state.present_velocity << "\t";
        out << "Pos. " << std::fixed << std::setw(6) << std::setprecision(2)
            << servo_state.present_position << "\t";
        out << "Volt. " << std::fixed << std::setw(6) << std::setprecision(2)
            << servo_state.voltage << "\t";
        out << "Temp. " << std::fixed << std::setw(6) << std::setprecision(2)
            << servo_state.temperature << "\r" << std::endl;
        return out;
    }

} // namespace platform

