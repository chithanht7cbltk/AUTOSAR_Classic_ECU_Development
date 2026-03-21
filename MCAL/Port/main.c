/**********************************************************
 * @file    main.c
 * @brief   Ứng dụng AUTOSAR MCAL: Dio, ADC, PWM + Port Driver
 * @details
 *  - PA0: DIO output, LED số, bật/tắt theo nút nhấn (PC13)
 *  - PC0: ADC input, đọc giá trị analog
 *  - PA8: PWM output, điều chỉnh độ sáng LED theo giá trị ADC
 *  - PC13: DIO input, nút nhấn (kéo xuống, nhấn = LOW)
 **********************************************************/

#include "Port.h"
#include "port_cfg.h"
#include "Dio.h"
#include "Adc.h" // Giả lập chuẩn AUTOSAR (bạn cần triển khai ADC chuẩn)
#include "Pwm.h" // Giả lập chuẩn AUTOSAR (bạn cần triển khai PWM chuẩn)
#include "stm32f10x.h"
#include <stdint.h>

/* Giá trị ngưỡng debounce (tùy vào tốc độ tick hệ thống) */
#define BUTTON_DEBOUNCE_DELAY 5000

int main(void)
{
    /* =======================
     * 1. Khởi tạo Port Driver
     * ======================= */
    Port_ConfigType portConfig = {
        .PinConfigs = PortCfg_Pins,
        .PinCount = PortCfg_PinsCount};
    Port_Init(&portConfig);

    /* =======================
     * 2. Khởi tạo các Driver ngoại vi
     * ======================= */
    Dio_Init(); // Ghi vào cho dễ hiểu chứ Dio không có Dio_Init, và phần cấu hình GPIO đã được viết trong Port_Init rồi
    Adc_Init(); // Bạn phải hiện thực chuẩn AUTOSAR
    Pwm_Init(); // Bạn phải hiện thực chuẩn AUTOSAR

    /* =======================
     * 3. Ứng dụng thực tế
     * ======================= */
    uint8_t led_state = 0;
    uint32_t button_cnt = 0;

    while (1)
    {
        /* ----- Đọc nút nhấn (PC13) để bật/tắt LED số (PA0) ----- */
        if (Dio_ReadChannel(DIO_CHANNEL(GPIOC, 13)) == STD_LOW)
        {
            /* Đơn giản debounce bằng bộ đếm */
            button_cnt++;
            if (button_cnt > BUTTON_DEBOUNCE_DELAY)
            {
                led_state = !led_state; // Đảo trạng thái
                Dio_WriteChannel(DIO_CHANNEL(GPIOA, 0), led_state ? STD_HIGH : STD_LOW);
                while (Dio_ReadChannel(DIO_CHANNEL(GPIOC, 13)) == STD_LOW)
                    ; // Đợi thả nút
                button_cnt = 0;
            }
        }
        else
        {
            button_cnt = 0;
        }

        /* ----- Đọc giá trị analog trên PC0 (ADC Channel 10) ----- */
        uint16_t adc_val = Adc_ReadChannel(ADC_Channel_10); // STM32F103: PC0 = ADC123_IN10

        /* ----- Điều chỉnh độ sáng LED PWM trên PA8 ----- */
        // Mapping giá trị ADC (0-4095) sang duty cycle PWM (0-1000)
        uint16_t duty = (adc_val * 1000) / 4095;
        Pwm_SetDutyCycle(PWM_CHANNEL_1, duty); // PWM_CHANNEL_1: Timer1 Channel1 → PA8
    }
}