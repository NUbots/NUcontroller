#ifndef NUSENSE_NUGUS_HPP
#define NUSENSE_NUGUS_HPP

#include <array>

#include "../dynamixel/DynamixelServo.hpp"
// #include "dynamixel/v2/FSR.hpp"

namespace nusense {

    struct MX64 : public dynamixel::DynamixelServo {
        MX64(uint8_t ID) : ID(ID) {}
        const uint8_t ID;
    };

    struct MX106 : public dynamixel::DynamixelServo {
        MX106(uint8_t ID) : ID(ID) {}
        const uint8_t ID;
    };

    struct XH540_W270 : public dynamixel::DynamixelServo {
        XH540_W270(uint8_t ID) : ID(ID) {}
        const uint8_t ID;
    };

    constexpr uint8_t NUMBER_OF_DEVICES = 20;

    class NUgus {
    public:
        NUgus();

        /// @brief The direction (clockwise or anticlockwise) to measure each motor in
        std::array<int8_t, 20> servo_direction{};

        /// @brief Offsets the radian angles of motors to change their 0 position
        std::array<double, 20> servo_offset{};

        enum class ID : uint8_t {
            NO_ID            = 0,
            R_SHOULDER_PITCH = 1,
            L_SHOULDER_PITCH = 2,
            R_SHOULDER_ROLL  = 3,
            L_SHOULDER_ROLL  = 4,
            R_ELBOW          = 5,
            L_ELBOW          = 6,
            R_HIP_YAW        = 7,
            L_HIP_YAW        = 8,
            R_HIP_ROLL       = 9,
            L_HIP_ROLL       = 10,
            R_HIP_PITCH      = 11,
            L_HIP_PITCH      = 12,
            R_KNEE           = 13,
            L_KNEE           = 14,
            R_ANKLE_PITCH    = 15,
            L_ANKLE_PITCH    = 16,
            R_ANKLE_ROLL     = 17,
            L_ANKLE_ROLL     = 18,
            HEAD_YAW         = 19,
            HEAD_PITCH       = 20,
            R_FSR            = 111,
            L_FSR            = 112,
            BROADCAST        = 254,
            MIN_ID           = R_SHOULDER_PITCH,
            MAX_ID           = L_FSR,
            MAX_SERVO_ID     = HEAD_PITCH
        };

        MX64 R_SHOULDER_PITCH;
        MX64 L_SHOULDER_PITCH;
        MX64 R_SHOULDER_ROLL;
        MX64 L_SHOULDER_ROLL;
        MX64 R_ELBOW;
        MX64 L_ELBOW;
        MX106 R_HIP_YAW;
        MX106 L_HIP_YAW;
        XH540_W270 R_HIP_ROLL;
        XH540_W270 L_HIP_ROLL;
        XH540_W270 R_HIP_PITCH;
        XH540_W270 L_HIP_PITCH;
        XH540_W270 R_KNEE;
        XH540_W270 L_KNEE;
        XH540_W270 R_ANKLE_PITCH;
        XH540_W270 L_ANKLE_PITCH;
        XH540_W270 R_ANKLE_ROLL;
        XH540_W270 L_ANKLE_ROLL;
        MX64 HEAD_YAW;
        MX64 HEAD_PITCH;

        /// @brief Get a reference to the DynamixelDevice with the given ID
        /// @param id The ID of the device to get
        /// @return A reference to the DynamixelDevice with the given ID
        constexpr dynamixel::DynamixelDevice& operator[](const ID& id) {
            switch (id) {
                case ID::R_SHOULDER_PITCH: return R_SHOULDER_PITCH;
                case ID::L_SHOULDER_PITCH: return L_SHOULDER_PITCH;
                case ID::R_SHOULDER_ROLL: return R_SHOULDER_ROLL;
                case ID::L_SHOULDER_ROLL: return L_SHOULDER_ROLL;
                case ID::R_ELBOW: return R_ELBOW;
                case ID::L_ELBOW: return L_ELBOW;
                case ID::R_HIP_YAW: return R_HIP_YAW;
                case ID::L_HIP_YAW: return L_HIP_YAW;
                case ID::R_HIP_ROLL: return R_HIP_ROLL;
                case ID::L_HIP_ROLL: return L_HIP_ROLL;
                case ID::R_HIP_PITCH: return R_HIP_PITCH;
                case ID::L_HIP_PITCH: return L_HIP_PITCH;
                case ID::R_KNEE: return R_KNEE;
                case ID::L_KNEE: return L_KNEE;
                case ID::R_ANKLE_PITCH: return R_ANKLE_PITCH;
                case ID::L_ANKLE_PITCH: return L_ANKLE_PITCH;
                case ID::R_ANKLE_ROLL: return R_ANKLE_ROLL;
                case ID::L_ANKLE_ROLL: return L_ANKLE_ROLL;
                case ID::HEAD_YAW: return HEAD_YAW;
                case ID::HEAD_PITCH:
                default: return HEAD_PITCH;
            }
        }

        /// @brief Get an array of the dynamixel IDs of each servo
        /// @warning Different to utility::input::ServoID which is zero indexed
        /// @return Array of uint8_t containing the IDs in order
        constexpr std::array<uint8_t, 20> servo_ids() const {
            return {uint8_t(ID::R_SHOULDER_PITCH), uint8_t(ID::L_SHOULDER_PITCH), uint8_t(ID::R_SHOULDER_ROLL),
                    uint8_t(ID::L_SHOULDER_ROLL),  uint8_t(ID::R_ELBOW),          uint8_t(ID::L_ELBOW),
                    uint8_t(ID::R_HIP_YAW),        uint8_t(ID::L_HIP_YAW),        uint8_t(ID::R_HIP_ROLL),
                    uint8_t(ID::L_HIP_ROLL),       uint8_t(ID::R_HIP_PITCH),      uint8_t(ID::L_HIP_PITCH),
                    uint8_t(ID::R_KNEE),           uint8_t(ID::L_KNEE),           uint8_t(ID::R_ANKLE_PITCH),
                    uint8_t(ID::L_ANKLE_PITCH),    uint8_t(ID::R_ANKLE_ROLL),     uint8_t(ID::L_ANKLE_ROLL),
                    uint8_t(ID::HEAD_YAW),         uint8_t(ID::HEAD_PITCH)};
        }
    };

    /// @brief The first part of the servo data to write to the dynamixel
    struct DynamixelServoWriteDataPart1 {
        uint8_t torque_enable;
        uint16_t velocity_i_gain;
        uint16_t velocity_p_gain;
        uint16_t position_d_gain;
        uint16_t position_i_gain;
        uint16_t position_p_gain;
    } __attribute__((packed));

    /// @brief The second part of the servo data to write to the dynamixel
    struct DynamixelServoWriteDataPart2 {
        uint16_t feedforward_1st_gain;
        uint16_t feedforward_2nd_gain;
        int16_t goal_pwm;
        int16_t goal_current;
        int32_t goal_velocity;
        uint32_t profile_acceleration;
        uint32_t profile_velocity;
        uint32_t goal_position;
    } __attribute__((packed));

    /// @brief The servo data to read from the dynamixel
    struct DynamixelServoReadData {
        uint8_t torque_enable;
        uint8_t hardware_error_status;
        int16_t present_pwm;
        int16_t present_current;
        int32_t present_velocity;
        uint32_t present_position;
        uint16_t present_voltage;
        uint8_t present_temperature;
    } __attribute__((packed));

    /// @brief  Document addresses used for read/writing to dynamixel devices, especially where
    ///         indirect addressing is used.
    enum class AddressBook : uint16_t {
        SERVO_READ_ADDRESS    = uint16_t(dynamixel::DynamixelServo::Address::INDIRECT_ADDRESS_1_L),
        SERVO_READ            = uint16_t(dynamixel::DynamixelServo::Address::INDIRECT_DATA_1),
        SERVO_WRITE_ADDRESS_1 = uint16_t(dynamixel::DynamixelServo::Address::INDIRECT_ADDRESS_18_L),
        SERVO_WRITE_ADDRESS_2 = uint16_t(dynamixel::DynamixelServo::Address::INDIRECT_ADDRESS_29_L),
        SERVO_WRITE_1         = uint16_t(dynamixel::DynamixelServo::Address::INDIRECT_DATA_18),
        SERVO_WRITE_2         = uint16_t(dynamixel::DynamixelServo::Address::INDIRECT_DATA_29)
        // FSR_READ              = uint16_t(FSR::Address::FSR1_L)
    };

}  // namespace nusense

#endif  // NUSENSE_NUGUS_HPP
