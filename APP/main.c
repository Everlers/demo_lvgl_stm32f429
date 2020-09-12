#include "stm32f4xx.h"
#include "bsp_debug_usart.h"
#include "bsp_general_tim.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"

#include "lv_examples.h"

int main (void)
{
	Debug_USART_Config();
	TIMx_Configuration();
	
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();
	
//	lv_ex_get_started_1();
//	lv_ex_get_started_2();
//	lv_ex_get_started_3();
	
//	lv_demo_keypad_encoder();
//	lv_demo_stress();
//	lv_demo_benchmark();
	lv_demo_widgets();
//	lv_demo_printer();
	while(1)
	{
		lv_task_handler();
	}
}

void TIM3_IRQHandler (void)
{
	lv_tick_inc(1);
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
}
