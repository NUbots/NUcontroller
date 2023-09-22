#include "main.h"       // needed for the GPIO labels and explicit types,
#include "usart.h"      // needed for UART handles,
#include "settings.h"   // needed for buzz during test,
#include "gpio.h"

#ifndef UART_RS485_H
#define UART_RS485_H

namespace uart {

    //#define DETECT_IDLE_LINE

    #define RS485_RX GPIO_PIN_RESET
    #define RS485_TX GPIO_PIN_SET

    class RS485 {
    public:
        /// @brief  the kind of status
        enum status {
            // Keep these values as they are to be compatible with HAL.
            RS485_OK       = 0x00,
            RS485_ERROR    = 0x01,
            RS485_BUSY     = 0x02,
            RS485_TIMEOUT  = 0x03
            // May add some more when the time comes.
        };
        /**
         * @brief   Maps the GPIO port and pin to the first UART interface.
         */
        RS485();
        /**
         * @brief   Maps the GPIO port and pin to the given UART interface.
         * @param   uart_number the number corresponding to the UART interface,
         */
        RS485(uint8_t uart_number);
        /**
         * @brief   Maps the GPIO port and pin to the given UART interface.
         * @param   input_huart the handle for the UART interface,
         */
        RS485(UART_HandleTypeDef* input_huart);
        /**
         * @brief   Destructs the link.
         * @note    Nothing needs to be freed.
         */
        virtual ~RS485();
        /**
         * @brief   Wraps the UART receive-function for RS485 hardware flow-control in polling-mode.
         * @note    The DXL direction pin is reset during this function.
         * @param   data the bytes to be sent,
         * @param   length the number of bytes, i.e. the length of the data,
         * @param   timeout the timeout in milliseconds, use HAL_MAX_DELAY for indefinite polling,
         * @return  the status of the UART,
         */
        status receive_poll(uint8_t* data, uint16_t length, uint32_t timeout);
        /**
         * @brief   Wraps the UART receive-function for RS485 hardware flow-control in interrupt-
         *          mode.
         * @note    The DXL direction pin is reset during this function.
         * @param   data the bytes to be sent,
         * @param   length the number of bytes, i.e. the length of the data,
         * @return  the status of the UART,
         */
        status receive_it(uint8_t* data, uint16_t length);

        /**
         * @brief   Wraps the UART receive-function for RS485 hardware flow-control in DMA-mode.
         * @note    The DXL direction pin is reset during this function.
         * @param   data the bytes to be sent,
         * @param   length the number of bytes, i.e. the length of the data,
         * @return  the status of the UART,
         */
        status receive(uint8_t* data, uint16_t length);

        /**
         * @brief   Checks for the interrupt-flags for the receiving to be done.
         * @return  whether something has been received,
         * @retval  #true if the receiving was done, i.e. something has been received,
         * @retval  #false if nothing has not been received yet,
         */
        bool get_receive_flag();

        /**
         * @brief   Gets the counter for the receiving DMA instance.
         * @return  the number of bytes yet to be received through the DMA before it is fully 
         *          complete,
         */
        uint16_t get_receive_counter();

        /**
         * @brief   Wraps the UART transmit-function for RS485 hardware flow-control in polling-
         *          mode.
         * @note    The DXL direction pin is set during this function.
         * @param   data the bytes to be sent,
         * @param   length the number of bytes, i.e. the length of the data,
         * @param   timeout the timeout in milliseconds, use HAL_MAX_DELAY for indefinite polling,
         * @return  the status of the UART,
         */
        status transmit_poll(const uint8_t* data, uint16_t length, uint32_t timeout);
        /**
         * @brief   Wraps the UART transmit-function for RS485 hardware flow-control in interrupt-
         *          mode.
         * @note    The DXL direction pin is set during this function.
         * @param   data the bytes to be sent,
         * @param   length the number of bytes, i.e. the length of the data,
         * @return  the status of the UART,
         */
        status transmit_it(const uint8_t* data, uint16_t length);

        /**
         * @brief   Wraps the UART transmit-function for RS485 hardware flow-control in DMA-mode.
         * @note    The DXL direction pin is set during this function.
         * @note    This should be used more preferably now that the bug has been fixed.
         * @param   data the bytes to be sent,
         * @param   length the number of bytes, i.e. the length of the data,
         * @return  the status of the UART,
         */
        status transmit(const uint8_t* data, uint16_t length);

        /**
         * @brief   Checks for the interrupt-flags for the transmitting to be done.
         * @note    The DXL direction pin is reset during this function if the flag has been set by 
         *          the interrupt.
         * @return  whether the data has been fully transmitted,
         * @retval  #true if the transmitting was done,
         * @retval  #false if not everything has been transmitted yet,
         */
        bool get_transmit_flag();

        /**
         * @brief   Gets the counter for the transmitting DMA instance.
         * @return  the number of bytes yet to be transmitted through the DMA before it is fully 
         *          complete,
         */
        uint16_t get_transmit_counter();
    private:
        /// @brief  the handle of the corresponding UART interface,
        UART_HandleTypeDef* huart;
        /// @brief  the handles of the corresponding DMA interfaces,
        DMA_HandleTypeDef* hdma_rx, * hdma_tx;
        /// @brief  the GPIO port of the direction-pin,
        GPIO_TypeDef* gpio_port;
        /// @brief  the GPIO pin of the direction-pin,
        uint16_t gpio_pin;
        /// @brief  the mask for the given UART interface's interrupt for receiving,
        uint16_t it_rx_mask;
        /// @brief  the mask for the given UART interface's interrupt for transmitting,
        uint16_t it_tx_mask;
    };

} // namespace uart

#endif // UART_RS485_H
