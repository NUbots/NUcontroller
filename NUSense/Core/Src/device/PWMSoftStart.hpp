#include "stm32h753xx.h"
#include "stm32h7xx_hal.c"
#define Tim2AF 0x01 //Timer 2 alternate function bit

#ifndef DEVICE_PWMSoftStart_HPP
    #define DEVICE_PWMSoftStart_HPP

namespace device {
   /**
    * @brief   The PWM soft start to avoid brown out of motors upon initialization
    * @note    All done in bare metal with the exception of the HAL_Delay using Timer 2 and the PB6("DXL_PWR_EN") pin
    *
    */
    class PWMSoftStart {
    public:
    	/**
    	 * @brief constructs PWMSoft.
    	 */
    	PWMSoftStart(){
    		Initialise();
    	}
    	/**
    	 * @brief   Destructs the PWMSoft.
    	 * @note    Nothing needs to be freed as of yet.
    	 */
        virtual ~PWMSoftStart() {}
        /**
         * @brief   Turns thePWM soft start on.
         * @note    May may add a slower start depending on the need with an incremental timer when starting up the pwm.
         */

        inline void start() {
        	TIM2->CR1|=TIM_CR1_CEN; //@brief enable TIM2

        	HAL_Delay(500); //@brief delay for 500 ms

        	TIM2->CCR1=1000; //@brief set PWM output to 100%

        }
    private:
        void initialise(){
        	/*
        	 * @brief initializes the PB6 pin and TIM2 to generate a PWM signal that can be turned on using the code
        	 */
        	RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN; //@brief Clock access to GPIOB enabled

        	GPIOB->MODER|=(GPIO_MODER_MODE6_1);//@brief Configure GPIOB on PB6 to alternate mode
        	GPIOB->MODER&=~(GPIO_MODER_MODE6_0);

        	GPIOB->AFR[0]|=(TIM2AF<<GPIO_AFRL_AFSEL6_POS); //@brief Set PB6 to Alternate function

        	RCC->APB2ENR|=RCC_APB2ENR_TIM2EN; //@brief enable clock access to TIM2

        	TIM2->PSC=15; //@brief Prescaler set to 15 to create a 1MHZ PWM signal

        	TIM2->ARR=1000; //@brief ARR set to 1000 to divide the PWM signal down to 1KHZ

        	TIM2->CCMR1|=TIM_CCMR1_OC1M_2; //@brief Configure ch1 to be pwm output

        	TIM2->CCER|=TIM_CCER_CC1E; //@brief enable ch1 as main output

        	TIM2->BDTR|=TIM_BDTR_MOE; //@brief start pwm output

        	TIM2->CCR1=500; //@brief set PWM output to 50%
        }

    };

}  // namespace device

#endif  // DEVICE_PWMSoftStart_HPP
