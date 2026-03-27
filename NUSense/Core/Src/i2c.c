#include "i2c.h"

I2C_HandleTypeDef hi2c3;

/**
 * @brief I2C3 Initialization Function
 * @param None
 * @retval None
 */
void MX_I2C3_Init(void)
{
    hi2c3.Instance = I2C3;
    hi2c3.Init.Timing = 0x00702681; // 400kHz I2C Fast Mode
    hi2c3.Init.OwnAddress1 = 0;
    hi2c3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c3.Init.OwnAddress2 = 0;
    hi2c3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c3) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c3, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c3, 0) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief I2C MSP Initialization
 * This function configures the hardware resources used by I2C3
 * @param hi2c: I2C handle pointer
 * @retval None
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hi2c->Instance == I2C3) {
        /* Peripheral clock enable */
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_I2C3_CLK_ENABLE();

        /**I2C3 GPIO Configuration
        PC9     ------> I2C3_SDA
        PA8     ------> I2C3_SCL
        */
        GPIO_InitStruct.Pin = I2C_SDA_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
        HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = I2C_SCL_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C3;
        HAL_GPIO_Init(I2C_SCL_GPIO_Port, &GPIO_InitStruct);
    }
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
    /* USER CODE BEGIN MX_GPIO_Init_1 */

    /* USER CODE END MX_GPIO_Init_1 */

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* I2C GPIO configuration is handled in HAL_I2C_MspInit */

    /* USER CODE BEGIN MX_GPIO_Init_2 */

    /* USER CODE END MX_GPIO_Init_2 */
}