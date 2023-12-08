#ifndef SIGNAL_H
#define SIGNAL_H

#include "main.h"

#define USE_SIGNALS
#ifdef USE_SIGNALS

#define SET_SIGNAL_1() SPARE1_GPIO_Port->BSRR = SPARE1_Pin;
#define RESET_SIGNAL_1() SPARE1_GPIO_Port->BSRR = (uint32_t) SPARE1_Pin << 16U;

#define SET_SIGNAL_2() SPARE2_GPIO_Port->BSRR = SPARE2_Pin;
#define RESET_SIGNAL_2() SPARE2_GPIO_Port->BSRR = (uint32_t) SPARE2_Pin << 16U;

#define SET_SIGNAL_3() SPARE3_GPIO_Port->BSRR = SPARE3_Pin;
#define RESET_SIGNAL_3() SPARE3_GPIO_Port->BSRR = (uint32_t) SPARE3_Pin << 16U;

#define SET_SIGNAL_4() SPARE4_GPIO_Port->BSRR = SPARE4_Pin;
#define RESET_SIGNAL_4() SPARE4_GPIO_Port->BSRR = (uint32_t) SPARE4_Pin << 16U;

#define SET_SIGNAL_5() SPARE5_GPIO_Port->BSRR = SPARE5_Pin;
#define RESET_SIGNAL_5() SPARE5_GPIO_Port->BSRR = (uint32_t) SPARE5_Pin << 16U;

#define SET_SIGNAL_6() SPARE6_GPIO_Port->BSRR = SPARE6_Pin;
#define RESET_SIGNAL_6() SPARE6_GPIO_Port->BSRR = (uint32_t) SPARE6_Pin << 16U;

#else

#define SET_SIGNAL_1() ;
#define RESET_SIGNAL_1() ;

#define SET_SIGNAL_2() ;
#define RESET_SIGNAL_2() ;

#define SET_SIGNAL_3() ;
#define RESET_SIGNAL_3() ;

#define SET_SIGNAL_4() ;
#define RESET_SIGNAL_4() ;

#define SET_SIGNAL_5() ;
#define RESET_SIGNAL_5() ;

#define SET_SIGNAL_6() ;
#define RESET_SIGNAL_6() ;

#endif

#endif
