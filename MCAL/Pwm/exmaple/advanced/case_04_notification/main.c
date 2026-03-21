/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 04: Notification + Output State
 * @details Mục tiêu:
 *          - Bật notification cho channel 0
 *          - Đọc output state và đưa output về idle theo chu kỳ
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile Pwm_OutputStateType g_case04_state = PWM_LOW;
volatile uint32 g_case04_tick = 0U;

int main(void)
{
    Pwm_Example_PortInit();
    Pwm_Init(&PwmDriverConfig);
    Pwm_EnableNotification(0U, PWM_BOTH_EDGES);

    while (1)
    {
        Pwm_SetDutyCycle(0U, 0x5000U);
        g_case04_state = Pwm_GetOutputState(0U);

        if ((g_case04_tick & 0x3FFU) == 0U)
        {
            Pwm_SetOutputToIdle(0U);
        }

        if ((g_case04_tick & 0x7FFU) == 0U)
        {
            Pwm_DisableNotification(0U);
        }
        else if ((g_case04_tick & 0x7FFU) == 0x400U)
        {
            Pwm_EnableNotification(0U, PWM_BOTH_EDGES);
        }

        g_case04_tick++;
        __asm volatile("nop");
    }
}
