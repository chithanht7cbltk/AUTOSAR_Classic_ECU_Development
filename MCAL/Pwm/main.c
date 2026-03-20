/**********************************************************
 * @file    main.c
 * @brief   Test AUTOSAR PWM Driver cho STM32F103 (2 channel, PA0 & PA7)
 * @author  HALA Academy
 **********************************************************/

#include "stm32f10x.h"
#include "Port.h"
#include "Port_cfg.h"
#include "Pwm.h"
#include "Pwm_Lcfg.h"
#include <stdint.h>

/* Hàm delay đơn giản cho test (blocking) */
static void DelayMs(uint32_t ms)
{
    volatile uint32_t n;
    for (; ms > 0; ms--) {
        for (n = 0; n < 8000; n++) __NOP();
    }
}

int main(void)
{
    /* ==== 1. Khởi tạo cấu hình Port chuẩn AUTOSAR ==== */
    Port_ConfigType portConfig = {
        .PinConfigs = PortCfg_Pins,
        .PinCount   = PortCfg_PinsCount
    };
    Port_Init(&portConfig);

    /* ==== 2. Khởi tạo PWM ==== */
    Pwm_Init(&PwmDriverConfig);

    /* ==== 3. Fade 2 channel PWM cùng lúc ==== */
    uint16_t dutyCh0 = 0;
    uint16_t dutyCh1 = 0;
    uint8_t dir0 = 1;
    uint8_t dir1 = 0;

    while (1)
    {
        /* Tăng giảm duty channel 0 (PA0 - TIM2_CH1) */
        Pwm_SetDutyCycle(0, dutyCh0);

        /* Tăng giảm duty channel 1 (PA7 - TIM3_CH2) */
        Pwm_SetDutyCycle(1, dutyCh1);

        DelayMs(10);

        /* Đổi hướng khi đạt 0 hoặc max */
        if (dir0) {
            dutyCh0 += 0x0200;
            if (dutyCh0 >= 0x8000) {
                dutyCh0 = 0x8000;
                dir0 = 0;
            }
        } else {
            if (dutyCh0 >= 0x0200) dutyCh0 -= 0x0200;
            else dutyCh0 = 0;
            if (dutyCh0 == 0) dir0 = 1;
        }

        /* Channel 1 thì làm ngược lại (fade lệch pha) */
        if (dir1) {
            dutyCh1 += 0x0200;
            if (dutyCh1 >= 0x8000) {
                dutyCh1 = 0x8000;
                dir1 = 0;
            }
        } else {
            if (dutyCh1 >= 0x0200) dutyCh1 -= 0x0200;
            else dutyCh1 = 0;
            if (dutyCh1 == 0) dir1 = 1;
        }
    }
}
