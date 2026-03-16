/**********************************************************
 * @file    main_combined.c
 * @brief   Example application using ADC with both IRQ and DMA
 * @details Đã được viết lại để phù hợp với kiến trúc 
 *          callback AUTOSAR mới, gọi chung tất cả Group.
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x.h"

volatile uint32_t g_eoc_irq_count = 0;
volatile uint32_t g_dma_tc_count = 0;

Adc_ValueGroupType myGroup0Buffer[2];
Adc_ValueGroupType myGroup1Buffer[2];

void MyAdcGroup0_Notification(void)
{
    g_eoc_irq_count++;
}

void MyAdcGroup1_DmaComplete(void)
{
    g_dma_tc_count++;
}

int main(void)
{
    Adc_ConfigType AdcConfig;
    Adc_Init(&AdcConfig);
    
    Adc_SetupResultBuffer(0, myGroup0Buffer);
    Adc_SetupResultBuffer(1, myGroup1Buffer);
    
    Adc_EnableGroupNotification(0); /* Enable NVIC EOC cho Group 0 */
    /* Group 1 tự động xài DMA NVIC vì đã config trong AdcGroupConfig */

    while (1) {
        Adc_StartGroupConversion(0);
        Adc_StartGroupConversion(1);
        
        for(volatile int i=0; i<50000; i++);
    }
    
    return 0;
}
