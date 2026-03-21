/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 05: Power state API
 * @details Mục tiêu:
 *          - Gọi Prepare/Set/GetCurrent/GetTarget power state
 *          - Lưu kết quả vào biến volatile để debug bằng GDB
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include "pwm_example_port.h"

volatile Pwm_PowerStateRequestResultType g_case05_result = PWM_SERVICE_ACCEPTED;
volatile Pwm_PowerStateType g_case05_current = PWM_FULL_POWER;
volatile Pwm_PowerStateType g_case05_target = PWM_FULL_POWER;
volatile Std_ReturnType g_case05_ret0 = E_NOT_OK;
volatile Std_ReturnType g_case05_ret1 = E_NOT_OK;
volatile Std_ReturnType g_case05_ret2 = E_NOT_OK;
volatile Std_ReturnType g_case05_ret3 = E_NOT_OK;

int main(void)
{
    Pwm_Example_PortInit();
    Pwm_Init(&PwmDriverConfig);

    while (1)
    {
        g_case05_ret0 = Pwm_PreparePowerState((Pwm_PowerStateType)1U, (Pwm_PowerStateRequestResultType *)&g_case05_result);
        g_case05_ret1 = Pwm_SetPowerState((Pwm_PowerStateRequestResultType *)&g_case05_result);
        g_case05_ret2 = Pwm_GetCurrentPowerState((Pwm_PowerStateType *)&g_case05_current, (Pwm_PowerStateRequestResultType *)&g_case05_result);
        g_case05_ret3 = Pwm_GetTargetPowerState((Pwm_PowerStateType *)&g_case05_target, (Pwm_PowerStateRequestResultType *)&g_case05_result);

        Pwm_Main_PowerTransitionManager();
        __asm volatile("nop");
    }
}
