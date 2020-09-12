#ifndef __GENERAL_TIM_H
#define	__GENERAL_TIM_H

#include "stm32f4xx.h"



#define GENERAL_TIM           			TIM3
#define GENERAL_TIM_CLK       		RCC_APB1Periph_TIM3

#define GENERAL_TIM_IRQn					TIM3_IRQn
#define GENERAL_TIM_IRQHandler 	TIM3_IRQHandler



void TIMx_Configuration(void);

#endif /* __GENERAL_TIM_H */

