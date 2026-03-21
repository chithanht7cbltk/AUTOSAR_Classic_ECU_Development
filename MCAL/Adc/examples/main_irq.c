/**********************************************************
 * @file    main_irq.c
 * @brief   Ví dụ ADC Group 0 dùng EOC IRQ notification
 * @details Ví dụ này minh họa luồng chuẩn:
 *          - Port_Init() cấu hình chân analog ở tầng Port Driver
 *          - Adc_Init() chỉ cấu hình ADC peripheral
 *          - Bật notification + start conversion bằng software trigger
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"

volatile Adc_ValueGroupType g_group0_buffer[4];
volatile uint32 g_irq_count = 0U;

/**********************************************************
 * @brief   Override callback weak cho Group 0
 **********************************************************/
void Adc_Group0_Notification(void)
{
    g_irq_count++;
}

int main(void)
{
    Adc_ValueGroupType readback[4];

    /* GPIO analog phải được setup ở Port Driver trước bước này */
    Adc_Init(&AdcDriverConfig);

    (void)Adc_SetupResultBuffer(0U, (Adc_ValueGroupType *)g_group0_buffer);
    Adc_EnableGroupNotification(0U);

    while (1)
    {
        Adc_StartGroupConversion(0U);
        (void)Adc_ReadGroup(0U, readback);

        for (volatile uint32 i = 0U; i < 100000U; i++)
        {
            __asm volatile("nop");
        }
    }
}
