/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 02: Quét duty cycle PWM
 * @details Mục tiêu:
 *          - Khởi tạo PWM
 *          - Quét duty cho channel 0 và channel 1
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile uint16 g_case02_last_duty_ch0 = 0U;
volatile uint16 g_case02_last_duty_ch1 = 0U;

int main(void)
{
    Pwm_Example_PortInit();
    uint16 duty = 0x0000U;

    Pwm_Init(&PwmDriverConfig);

    while (1)
    {
        Pwm_SetDutyCycle(0U, duty);
        Pwm_SetDutyCycle(1U, (uint16)(0x8000U - duty));
        g_case02_last_duty_ch0 = duty;
        g_case02_last_duty_ch1 = (uint16)(0x8000U - duty);

        duty = (uint16)(duty + 0x0800U);
        if (duty > 0x8000U)
        {
            duty = 0x0000U;
        }

        __asm volatile("nop");
    }
}
