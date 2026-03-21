/**********************************************************
 * @file    pwm_api_test_main.c
 * @brief   Test harness cho PWM API chạy trên Renode + GDB
 * @details Chương trình chính giữ CPU ở vòng lặp để GDB
 *          điều khiển gọi API và kiểm tra kết quả.
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Lcfg.h"

/* ===========================================
 *  Biến toàn cục phục vụ quan sát bằng GDB
 * =========================================== */
volatile Std_VersionInfoType g_version_info;
volatile Pwm_PowerStateRequestResultType g_power_result = PWM_SERVICE_ACCEPTED;
volatile Pwm_PowerStateType g_power_state = PWM_FULL_POWER;
volatile uint32 g_test_magic = 0U;

/**********************************************************
 * @brief   Điểm vào chương trình test harness
 * @details Không chạy testcase trong main, mọi thao tác test
 *          sẽ do GDB điều khiển qua lệnh call/print.
 **********************************************************/
int main(void)
{
    while (1)
    {
        g_test_magic++;
        __asm volatile("nop");
    }
}
