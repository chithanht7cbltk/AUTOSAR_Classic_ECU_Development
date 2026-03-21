/**********************************************************
 * @file    pwm_example_port.c
 * @brief   Port Init dùng chung cho PWM exmaple
 * @details Cấu hình GPIO alternate function push-pull cho các chân PWM.
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "pwm_example_port.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/**********************************************************
 * @brief   Cấu hình chân GPIO cho PWM output
 * @details Chỉ xử lý phần chân:
 *          - PA0: TIM2_CH1
 *          - PA7: TIM3_CH2
 *          Việc cấu hình timer vẫn do Pwm_Init() đảm nhiệm.
 **********************************************************/
void Pwm_Example_PortInit(void)
{
    GPIO_InitTypeDef gpioCfg;

    /* Bật clock AFIO và GPIOA để dùng alternate function output */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    /* PA0 + PA7 ở mode AF Push-Pull cho output PWM */
    gpioCfg.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_7;
    gpioCfg.GPIO_Speed = GPIO_Speed_50MHz;
    gpioCfg.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &gpioCfg);
}
