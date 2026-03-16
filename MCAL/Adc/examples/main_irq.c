/**********************************************************
 * @file    main_irq.c
 * @brief   Example application using ADC with Interrupts
 * @details This example demonstrates how to configure and use
 *          the ADC driver with End Of Conversion (EOC) Interrupts
 *          based on the new AUTOSAR pattern.
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x.h"

/* Buffer nhận dữ liệu ADC */
Adc_ValueGroupType myGroup0Buffer[2];

int main(void)
{
    /* PortInit() - Giả định Port đã config GPIO */
    
    /* 1. Init ADC với cấu hình (dummy config vì dùng extern array AdcGroupConfig) */
    Adc_ConfigType AdcConfig;
    Adc_Init(&AdcConfig);
    
    /* 2. Setup buffer cho group 0 */
    Adc_SetupResultBuffer(0, myGroup0Buffer);
    
    /* 3. Enable notification và start conversion */
    Adc_EnableGroupNotification(0);

    while (1) {
        /* Bắt đầu ADC (Thay thế cho hardware timer trong ví dụ này) */
        Adc_StartGroupConversion(0);
        
        /* Delay loop giả lập */
        for(volatile int i=0; i<500000; i++);
    }
    
    return 0;
}
