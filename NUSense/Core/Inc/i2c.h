#ifndef I2C_H
#define I2C_H

#include "main.h"
#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define I2C_SDA_Pin GPIO_PIN_9
#define I2C_SDA_GPIO_Port GPIOC
#define I2C_SCL_Pin GPIO_PIN_8
#define I2C_SCL_GPIO_Port GPIOA

extern I2C_HandleTypeDef hi2c3;

void MX_I2C3_Init(void);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);

#ifdef __cplusplus
}
#endif

#endif // I2C_H

