#ifndef PLATFORM_NUSENSE_NUSENSESTATE_HPP
#define PLATFORM_NUSENSE_NUSENSESTATE_HPP

namespace platform::NUsense {

    /// @brief  The state of the hardware on the board.
    /// @note   May be used later on for IO, etc.
    struct NUsenseState {
        /// @brief  the model-number, i.e. the revision of the PCB,
        uint8_t model = 0x00;
        /// @brief  the firmware-version,
        uint8_t firmware = 0x00;
        /// @brief  the state of LED_RX on the back-panel,
        bool led_rx = false;
        /// @brief  the state of LED_TX on the back-panel,
        bool led_tx = false;
        /// @brief  the state of LED2 on the back-panel,
        bool led_2 = false;
        /// @brief  the state of LED3 on the back-panel,
        bool led_3 = false;
        /// @brief  the state of LED4 on the back-panel,
        bool led_4 = false;
        /// @brief  the RGB state of LED5 on the back-panel,
        RgbLed led_5;
        /// @brief  the RGB state of LED6 on the back-panel,
        RgbLed led_6;
        /// @brief  the state of the buzzer,
        /// @note   We may later make a class that can control PWM, beeps, etc.
        bool buzzer; 
        /// @brief  the state of the Dynamixel power,
        bool dxl_power = true;
        /// @brief  the supply-voltage,
        float voltage = 0.0f;
    };

    struct RgbLed {
        uint8_t red = 0x00;
        uint8_t green = 0x00;
        uint8_t blue = 0x00;
    };

} // namespace platform::NUsense

#endif  // PLATFORM_NUSENSE_NUSENSESTATE_HPP
