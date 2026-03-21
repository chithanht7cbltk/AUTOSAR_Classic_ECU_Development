/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 08: Full API smoke test
 * @details Mục tiêu:
 *          - Chạy chuỗi gọi API chính của PWM trong một vòng lặp
 *          - Lưu trạng thái để dễ trace khi debug trên Renode/GDB
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile Std_VersionInfoType g_case08_ver;
volatile Pwm_OutputStateType g_case08_state0 = PWM_LOW;
volatile Pwm_OutputStateType g_case08_state1 = PWM_LOW;
volatile Pwm_PowerStateRequestResultType g_case08_power_result = PWM_SERVICE_ACCEPTED;
volatile Pwm_PowerStateType g_case08_power_current = PWM_FULL_POWER;
volatile Pwm_PowerStateType g_case08_power_target = PWM_FULL_POWER;
volatile uint32 g_case08_tick = 0U;

int main(void)
{
    Pwm_Example_PortInit();
    Pwm_Init(&PwmDriverConfig);
    Pwm_GetVersionInfo((Std_VersionInfoType *)&g_case08_ver);

    while (1)
    {
        Pwm_SetDutyCycle(0U, 0x3000U);
        Pwm_SetDutyCycle(1U, 0x6000U);
        Pwm_SetPeriodAndDuty(0U, 1500U, 0x4000U);

        Pwm_EnableNotification(0U, PWM_BOTH_EDGES);
        g_case08_state0 = Pwm_GetOutputState(0U);
        g_case08_state1 = Pwm_GetOutputState(1U);

        if ((g_case08_tick & 0x1FFU) == 0U)
        {
            Pwm_SetOutputToIdle(1U);
        }

        (void)Pwm_PreparePowerState((Pwm_PowerStateType)1U, (Pwm_PowerStateRequestResultType *)&g_case08_power_result);
        (void)Pwm_SetPowerState((Pwm_PowerStateRequestResultType *)&g_case08_power_result);
        (void)Pwm_GetCurrentPowerState((Pwm_PowerStateType *)&g_case08_power_current, (Pwm_PowerStateRequestResultType *)&g_case08_power_result);
        (void)Pwm_GetTargetPowerState((Pwm_PowerStateType *)&g_case08_power_target, (Pwm_PowerStateRequestResultType *)&g_case08_power_result);
        Pwm_Main_PowerTransitionManager();

        if ((g_case08_tick & 0x3FFU) == 0x200U)
        {
            Pwm_DisableNotification(0U);
        }

        g_case08_tick++;
        __asm volatile("nop");
    }
}
