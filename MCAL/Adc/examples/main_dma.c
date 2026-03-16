/**********************************************************
 * @file    main_dma.c
 * @brief   Example application using ADC with DMA Support
 * @details Đã được viết lại để phù hợp với kiến trúc 
 *          callback AUTOSAR mới của Adc_Cfg Group 1.
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x.h"

/* Buffer nhận dữ liệu ADC (2 kênh, mỗi kênh 1 value) */
Adc_ValueGroupType myGroup1Buffer[2];

int main(void)
{
    /* Bỏ qua Port_Init() GPIO do giả lập AUTOSAR */
    
    /* 1. Init ADC. Sẽ quét AdcGroupConfig[] để khởi tạo cả Group 0 và 1 */
    Adc_ConfigType AdcConfig;
    Adc_Init(&AdcConfig);
    
    /* 2. Setup DMA target memory cho group 1 
       Lưu ý: Group 1 theo Adc_Cfg.c đang là DMA Mode, 2 kênh */
    Adc_SetupResultBuffer(1, myGroup1Buffer);
    
    /* (Group 1 notification cho DMA không cần EnableGroupNotification như EOC) */

    while (1) {
        /* 3. Phát xung Start ADC (Do cấu hình Software Trigger hoặc None) */
        Adc_StartGroupConversion(1);
        
        /* Chờ quá trình DMA diễn ra */
        for(volatile int i=0; i<500000; i++);
    }
    
    return 0;
}
