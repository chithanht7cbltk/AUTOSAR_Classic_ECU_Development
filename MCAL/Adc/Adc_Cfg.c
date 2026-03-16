#include "Adc_Cfg.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include <stddef.h>

/* ===============================
 *  Group Callbacks Declarations
 * =============================== */
extern void MyAdcGroup0_Notification(void);
extern void MyAdcGroup1_DmaComplete(void);

void ADC1_2_IRQHandler(void)
{
    extern void Adc_IsrHandler(ADC_TypeDef* ADCx);
    Adc_IsrHandler(ADC1);
}

/* ===============================
 *  Group 1 Callbacks & Cfg (DMA Mode)
 * =============================== */

void DMA1_Channel1_IRQHandler(void)
{
    extern void Adc_DmaIsrHandler(DMA_Channel_TypeDef* DMAy_Channelx);
    /* Gửi signal tương ứng channel DMA đang nối với ADC1 là Channel 1 */
    Adc_DmaIsrHandler(DMA1_Channel1);
}

static const Adc_DmaConfigType DMA_Group1_Cfg = {
    .dmaChannel = DMA1_Channel1,
    .priority = DMA_Priority_High,
    .bufferSize = 2, // 2 Channels
    .mode = ADC_STREAM_BUFFER_CIRCULAR
};

/* ===============================
 *  Global Group Config Array
 * =============================== */
const Adc_GroupConfigType AdcGroupConfig[ADC_NUM_GROUPS] = {
    { 
        .ADCx = ADC1, 
        .groupId = 0, 
        .channelCount = 1, 
        .triggerType = ADC_TRIGGER_EOC_INT,
        .dmaConfig = NULL,
        .channels = { {ADC_Channel_0, ADC_SAMPLETIME_55CYCLES_5, 1} },
        .NotificationCb = MyAdcGroup0_Notification,
        .DmaHalfCb = NULL,
        .DmaCompleteCb = NULL
    },
    { 
        .ADCx = ADC1, 
        .groupId = 1, 
        .channelCount = 2, 
        .triggerType = ADC_TRIGGER_DMA_CIRCULAR,
        .dmaConfig = &DMA_Group1_Cfg,
        .channels = { 
            {ADC_Channel_0, ADC_SAMPLETIME_55CYCLES_5, 1},
            {ADC_Channel_1, ADC_SAMPLETIME_55CYCLES_5, 2}
        },
        .NotificationCb = NULL,
        .DmaHalfCb = NULL,
        .DmaCompleteCb = MyAdcGroup1_DmaComplete
    }
};
