/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "dma.h"
#include "gpio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
#include "nusense/NUSenseIO.hpp"
#include "settings.h"
#include "test_hw.hpp"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();

    /* Configure the system clock */
    SystemClock_Config();

    // NOTE: make sure that MX_DMA_Init is called before everything else. CubeMX has a strange habit
    // of doing things in the wrong order. This is a known bug, that has not been updated strangely
    // enough in version 6.6.0. It should not matter too much anyway; CubeMX should not change the
    // order here given that it also has another strange habit of ignoring main.cpp and creating and
    // overwriting main.c instead despite CubeIDE knowing that this is a C++ project! Oh CubeMX ...

    /* Initialize all configured peripherals */
    MX_DMA_Init();
    MX_GPIO_Init();
    MX_USB_DEVICE_Init();
    MX_SPI4_Init();
    MX_UART4_Init();
    MX_UART5_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_USART6_UART_Init();
    MX_TIM1_Init();
    MX_TIM4_Init();

#ifdef FIRST_BUZZ
    // Confirm that the programme is running.
    HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);
#endif

    /// @note Setting the DXL_PWR pin should not be done here, but at the moment it is because
    /// otherwise, the NUC will brown out when a binary is running on it and NUSense tries to set this pin.
    /// This means that our "Emergency Stop" works by turning the DXL power on anyway at boot BUT it will not process
    /// any servo packets until it receives a valid handshake message from the NUC. This has to be rectified as an ideal
    /// E-Stop requires the power to be cut off and not turned on until we know we can turn it on.
    HAL_GPIO_WritePin(DXL_PWR_EN_GPIO_Port, DXL_PWR_EN_Pin, GPIO_PIN_SET);

    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

    // Enable the clock for GPIOH.
    RCC->AHB4ENR |= (0b1 << (7));

#ifdef RUN_MAIN
    nusense::NUSenseIO nusenseIO;

    // Wait for the initial handshake message from NUC
    while (!nusenseIO.handshake_received()) {
    }
    // Buzz after the handshake has been received
    HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(BUZZER_SIG_GPIO_Port, BUZZER_SIG_Pin, GPIO_PIN_RESET);

    // Delay for a bit to give the motor time to boot up. Without this delay, I found that the
    // motor does not respond at all. From some basic testing, I think that it is because the motor
    // only boots up until the DXL power is switched on from the DXL_POWER_EN pin. However, it may
    // have something to do with the RS485 transceivers instead.
    HAL_Delay(1000);

    nusenseIO.startup();

    /*
    ~~~ ~~~ ~~~ The Main Loop ~~~ ~~~ ~~~
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    */
    while (1) {

        nusenseIO.loop();
    }

#else
    #ifdef TEST_IMU
    test_hw::imu();
    #endif
#endif  // RUN_MAIN
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Supply configuration update enable
     */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
    }

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
    }

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI48 | RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.HSI48State          = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = 4;
    RCC_OscInitStruct.PLL.PLLN            = 60;
    RCC_OscInitStruct.PLL.PLLP            = 2;
    RCC_OscInitStruct.PLL.PLLQ            = 2;
    RCC_OscInitStruct.PLL.PLLR            = 2;
    RCC_OscInitStruct.PLL.PLLRGE          = RCC_PLL1VCIRANGE_3;
    RCC_OscInitStruct.PLL.PLLVCOSEL       = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN        = 0;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2
                                  | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.SYSCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
    RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
    /* User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
