/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 06: Kịch bản channel không hợp lệ
 * @details Mục tiêu:
 *          - Truyền channel id sai để kiểm tra tính an toàn của API
 *          - Quan sát output state trả về và trạng thái version info
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile Pwm_OutputStateType g_case06_state_invalid = PWM_LOW;
volatile Std_VersionInfoType g_case06_version;

int main(void)
{
    Pwm_Example_PortInit();
    Pwm_Init(&PwmDriverConfig);

    while (1)
    {
        Pwm_SetDutyCycle(99U, 0x8000U);
        Pwm_SetPeriodAndDuty(99U, 2000U, 0x4000U);
        Pwm_SetOutputToIdle(99U);
        Pwm_DisableNotification(99U);
        Pwm_EnableNotification(99U, PWM_BOTH_EDGES);

        g_case06_state_invalid = Pwm_GetOutputState(99U);
        Pwm_GetVersionInfo((Std_VersionInfoType *)&g_case06_version);
        __asm volatile("nop");
    }
}
