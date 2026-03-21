/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 03: Điều khiển period và duty
 * @details Mục tiêu:
 *          - Dùng Pwm_SetPeriodAndDuty() trên kênh variable period
 *          - Tạo biến period/duty để quan sát trên debugger
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile Pwm_PeriodType g_case03_period = 1000U;
volatile uint16 g_case03_duty = 0x2000U;

int main(void)
{
    Pwm_Example_PortInit();
    Pwm_Init(&PwmDriverConfig);

    while (1)
    {
        Pwm_SetPeriodAndDuty(0U, g_case03_period, g_case03_duty);

        g_case03_period = (Pwm_PeriodType)(g_case03_period + 100U);
        if (g_case03_period > 2500U)
        {
            g_case03_period = 1000U;
        }

        g_case03_duty = (uint16)(g_case03_duty + 0x0400U);
        if (g_case03_duty > 0x7000U)
        {
            g_case03_duty = 0x1000U;
        }

        __asm volatile("nop");
    }
}
