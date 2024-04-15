#include "RS485.h"

// Higher-level interrupt flags for easier set-up.
// Maybe be neater to have it as a static variable with a getter.
static volatile uint16_t uart_it_flags;

/**
 * @brief   Sets the appropriate flag based on the interrupted UART on all data being received.
 * @param   handle the handle for the interrupted UART,
 * @return  nothing
 */
#ifdef DETECT_IDLE_LINE
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* handle, uint16_t size) {
#else
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* handle) {
#endif
    if      (handle == &huart1) uart_it_flags |= UART1_RX;
    else if (handle == &huart2) uart_it_flags |= UART2_RX;
    else if (handle == &huart3) uart_it_flags |= UART3_RX;
    else if (handle == &huart4) uart_it_flags |= UART4_RX;
    else if (handle == &huart5) uart_it_flags |= UART5_RX;
    else if (handle == &huart6) uart_it_flags |= UART6_RX;
}
/**
 * @brief   Sets the appropriate flag based on the interrupted UART on all data being transmitted.
 * @param   handle the handle for the interrupted UART,
 * @return  nothing
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* handle) {
    if (handle == &huart1) {
        uart_it_flags |= UART1_TX;
        HAL_GPIO_WritePin(DXL_DIR1_GPIO_Port, DXL_DIR1_Pin, RS485_RX);
    }
    else if (handle == &huart2) {
        uart_it_flags |= UART2_TX;
        HAL_GPIO_WritePin(DXL_DIR2_GPIO_Port, DXL_DIR2_Pin, RS485_RX);
    }
    else if (handle == &huart3) {
        uart_it_flags |= UART3_TX;
        HAL_GPIO_WritePin(DXL_DIR3_GPIO_Port, DXL_DIR3_Pin, RS485_RX);
    }
    else if (handle == &huart4) {
        uart_it_flags |= UART4_TX;
        HAL_GPIO_WritePin(DXL_DIR4_GPIO_Port, DXL_DIR4_Pin, RS485_RX);
    }
    else if (handle == &huart5) {
        uart_it_flags |= UART5_TX;
        HAL_GPIO_WritePin(DXL_DIR5_GPIO_Port, DXL_DIR5_Pin, RS485_RX);
    }
    else if (handle == &huart6) {
        uart_it_flags |= UART6_TX;
        HAL_GPIO_WritePin(DXL_DIR6_GPIO_Port, DXL_DIR6_Pin, RS485_RX);
    }
}

namespace uart {

    RS485::RS485() {
        huart = &huart1;
        hdma_rx = &hdma_usart1_rx;
        // Map the GPIO port and pin to the correct one corresponding to the given UART interface.
        gpio_port = DXL_DIR1_GPIO_Port; 
        gpio_pin = DXL_DIR1_Pin; 
        it_rx_mask = UART1_RX; 
        it_tx_mask = UART1_TX;
    }
    RS485::RS485(uint8_t uart_number) {
        // Map the GPIO port and pin to the correct one corresponding to the given UART interface.
        switch (uart_number) {
        case 1:
            huart = &huart1; hdma_rx = &hdma_usart1_rx; hdma_tx = &hdma_usart1_tx;
            gpio_port = DXL_DIR1_GPIO_Port; gpio_pin = DXL_DIR1_Pin;
            it_rx_mask = UART1_RX; it_tx_mask = UART1_TX;
            break;
        case 2:
            huart = &huart2; hdma_rx = &hdma_usart2_rx; hdma_tx = &hdma_usart2_tx;
            gpio_port = DXL_DIR2_GPIO_Port; gpio_pin = DXL_DIR2_Pin;
            it_rx_mask = UART2_RX; it_tx_mask = UART2_TX;
            break;
        case 3:
            huart = &huart3; hdma_rx = &hdma_usart3_rx; hdma_tx = &hdma_usart3_tx;
            gpio_port = DXL_DIR3_GPIO_Port; gpio_pin = DXL_DIR3_Pin;
            it_rx_mask = UART3_RX; it_tx_mask = UART3_TX;
            break;
        case 4:
            huart = &huart4; hdma_rx = &hdma_uart4_rx;     hdma_tx = &hdma_uart4_tx;
            gpio_port = DXL_DIR4_GPIO_Port; gpio_pin = DXL_DIR4_Pin;
            it_rx_mask = UART4_RX; it_tx_mask = UART4_TX;
            break;
        case 5:
            huart = &huart5; hdma_rx = &hdma_uart5_rx;     hdma_tx = &hdma_uart5_tx;
            gpio_port = DXL_DIR5_GPIO_Port; gpio_pin = DXL_DIR5_Pin;
            it_rx_mask = UART5_RX; it_tx_mask = UART5_TX;
            break;
        case 6:
            huart = &huart6; hdma_rx = &hdma_usart6_rx; hdma_tx = &hdma_usart6_tx;
            gpio_port = DXL_DIR6_GPIO_Port; gpio_pin = DXL_DIR6_Pin;
            it_rx_mask = UART6_RX; it_tx_mask = UART6_TX;
            break;
        }
    }


    RS485::RS485(UART_HandleTypeDef* input_huart) {
        huart = input_huart;
        // Map the GPIO port and pin to the correct one corresponding to the given UART interface.
        if (huart == &huart1){
            hdma_rx = &hdma_usart1_rx; hdma_tx = &hdma_usart1_tx;
            gpio_port = DXL_DIR1_GPIO_Port; gpio_pin = DXL_DIR1_Pin;
            it_rx_mask = UART1_RX; it_tx_mask = UART1_TX;
        } else if (huart == &huart2) {
            hdma_rx = &hdma_usart2_rx; hdma_tx = &hdma_usart2_tx;
            gpio_port = DXL_DIR2_GPIO_Port; gpio_pin = DXL_DIR2_Pin;
            it_rx_mask = UART2_RX; it_tx_mask = UART2_TX;
        } else if (huart == &huart3) {
            hdma_rx = &hdma_usart3_rx; hdma_tx = &hdma_usart3_tx;
            gpio_port = DXL_DIR3_GPIO_Port; gpio_pin = DXL_DIR3_Pin;
            it_rx_mask = UART3_RX; it_tx_mask = UART3_TX;
        } else if (huart == &huart4) {
            hdma_rx = &hdma_uart4_rx; hdma_tx = &hdma_uart4_tx;
            gpio_port = DXL_DIR4_GPIO_Port; gpio_pin = DXL_DIR4_Pin;
            it_rx_mask = UART4_RX; it_tx_mask = UART4_TX;
        } else if (huart == &huart5) {
            hdma_rx = &hdma_uart5_rx; hdma_tx = &hdma_uart5_tx;
            gpio_port = DXL_DIR5_GPIO_Port; gpio_pin = DXL_DIR5_Pin;
            it_rx_mask = UART5_RX; it_tx_mask = UART5_TX;
        } else if (huart == &huart6) {
            hdma_rx = &hdma_usart6_rx; hdma_tx = &hdma_usart6_tx;
            gpio_port = DXL_DIR6_GPIO_Port; gpio_pin = DXL_DIR6_Pin;
            it_rx_mask = UART6_RX; it_tx_mask = UART6_TX;
        }
    }

    RS485::~RS485() {
        // May do something with this later.
    }

    RS485::status RS485::receive_poll(uint8_t* data, uint16_t length, uint32_t timeout) {
        // Declare local variables.
        RS485::status status;

        // Set the hardware to the receiving direction and listen.
        HAL_GPIO_WritePin(gpio_port, gpio_pin, RS485_RX);
        status = (RS485::status)HAL_UART_Receive(huart, data, length, timeout);
    #ifdef TEST_UART
        // If this is during a test, then play buzzer when there is an error.
        if (RS485_OK != status) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }
    #endif

        return status;
    }

    RS485::status RS485::receive_it(uint8_t* data, uint16_t length) {
        // Declare local variables.
        RS485::status status;

        // Set the hardware to the receiving direction and listen.
        HAL_GPIO_WritePin(gpio_port, gpio_pin, RS485_RX);
        status = (RS485::status)HAL_UART_Receive_IT(huart, data, length);
    #ifdef TEST_UART
        // If this is during a test, then play buzzer when there is an error.
        if (RS485_OK != status) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }
    #endif

        return status;
    }

    RS485::status RS485::receive(uint8_t* data, uint16_t length) {
        // Declare local variables.
        RS485::status status;

        // Set the hardware to the receiving direction and listen.
        HAL_GPIO_WritePin(gpio_port, gpio_pin, RS485_RX);
    #ifdef DETECT_IDLE_LINE
        status = (RS485::status)HAL_UARTEx_ReceiveToIdle_DMA(huart, data, length+3);
        __HAL_DMA_DISABLE_IT(hdma_rx, DMA_IT_HT);
    #else
        status = (RS485::status)HAL_UART_Receive_DMA(huart, data, length);
    #endif

    #ifdef TEST_UART
        // If this is during a test, then play buzzer when there is an error.
        if (RS485_OK != status) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }
    #endif
        return status;
    }

    bool RS485::get_receive_flag() {
        if (uart_it_flags & it_rx_mask) {
            uart_it_flags &= ~it_rx_mask;
            return true;
        } else
            return false;
    }

    uint16_t RS485::get_receive_counter() {
        return __HAL_DMA_GET_COUNTER(hdma_rx);
    }

    RS485::status RS485::transmit_poll(const uint8_t* data, uint16_t length, uint32_t timeout) {
        // Declare local variables.
        RS485::status status;

        // Set the hardware to the receiving direction and send.
        HAL_GPIO_WritePin(gpio_port, gpio_pin, RS485_TX);
        status = (RS485::status)HAL_UART_Transmit(huart, data, length, timeout);
    #ifdef TEST_UART
        // If this is during a test, then play buzzer when there is an error.
        if (RS485_OK != status) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }
    #endif

        return status;
    }

    RS485::status RS485::transmit_it(const uint8_t* data, uint16_t length) {
        // Declare local variables.
        RS485::status status;

        // Set the hardware to the receiving direction and send.
        HAL_GPIO_WritePin(gpio_port, gpio_pin, RS485_TX);
        status = (RS485::status)HAL_UART_Transmit_IT(huart, data, length);
    #ifdef TEST_UART
        // If this is during a test, then play buzzer when there is an error.
        if (RS485_OK != status) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }
    #endif

        return status;
    }

    RS485::status RS485::transmit(const uint8_t* data, uint16_t length) {
        // Declare local variables.
        RS485::status status;

        // Set the hardware to the receiving direction and send.
        HAL_GPIO_WritePin(gpio_port, gpio_pin, RS485_TX);
        status = (RS485::status)HAL_UART_Transmit_DMA(huart, data, length);
    #ifdef TEST_UART
        // If this is during a test, then play buzzer when there is an error.
        if (RS485_OK != status) {
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
            HAL_Delay(100);
            HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
        }
    #endif

        return status;
    }

    bool RS485::get_transmit_flag() {
        if (uart_it_flags & it_tx_mask) {
            uart_it_flags &= ~it_tx_mask;
            return true;
        } else
            return false;
    }

    uint16_t RS485::get_transmit_counter() {
        return __HAL_DMA_GET_COUNTER(hdma_tx);
    }

} // namespace uart
