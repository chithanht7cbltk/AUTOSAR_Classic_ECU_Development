/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 01: Khởi tạo và de-init PWM
 * @details Mục tiêu:
 *          - Gọi Pwm_Init() với cấu hình toàn cục
 *          - Gọi Pwm_DeInit() để đưa module về trạng thái ban đầu
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile uint32 g_case01_counter = 0U;

int main(void)
{
    Pwm_Example_PortInit();
    Pwm_Init(&PwmDriverConfig);
    Pwm_DeInit();

    while (1)
    {
        g_case01_counter++;
        __asm volatile("nop");
    }
}
