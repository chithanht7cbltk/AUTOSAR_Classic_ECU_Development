/**********************************************************
 * @file    main.c
 * @brief   Exmaple case 07: Power API khi chưa init
 * @details Mục tiêu:
 *          - Không gọi Pwm_Init()
 *          - Kiểm tra behavior các API power state ở trạng thái uninit
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "pwm_example_port.h"

volatile Pwm_PowerStateRequestResultType g_case07_result = PWM_SERVICE_ACCEPTED;
volatile Pwm_PowerStateType g_case07_state = PWM_FULL_POWER;
volatile Std_ReturnType g_case07_ret0 = E_NOT_OK;
volatile Std_ReturnType g_case07_ret1 = E_NOT_OK;
volatile Std_ReturnType g_case07_ret2 = E_NOT_OK;
volatile Std_ReturnType g_case07_ret3 = E_NOT_OK;

int main(void)
{
    Pwm_Example_PortInit();
    while (1)
    {
        g_case07_ret0 = Pwm_PreparePowerState((Pwm_PowerStateType)1U, (Pwm_PowerStateRequestResultType *)&g_case07_result);
        g_case07_ret1 = Pwm_SetPowerState((Pwm_PowerStateRequestResultType *)&g_case07_result);
        g_case07_ret2 = Pwm_GetCurrentPowerState((Pwm_PowerStateType *)&g_case07_state, (Pwm_PowerStateRequestResultType *)&g_case07_result);
        g_case07_ret3 = Pwm_GetTargetPowerState((Pwm_PowerStateType *)&g_case07_state, (Pwm_PowerStateRequestResultType *)&g_case07_result);

        Pwm_Main_PowerTransitionManager();
        __asm volatile("nop");
    }
}
