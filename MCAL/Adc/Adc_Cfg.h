#ifndef ADC_CFG_H
#define ADC_CFG_H

#include "Adc.h"

typedef struct {
    ADC_TypeDef*             ADCx;           // ADC instance (ADC1/ADC2)
    Adc_GroupType            groupId;        // Group numeric ID
    uint8_t                  channelCount;   // Số kênh trong group
    Adc_TriggerType          triggerType;    // Loại trigger kích hoạt ADC
    const Adc_DmaConfigType* dmaConfig;      // Cấu hình con cho DMA (nếu có)
    Adc_ChannelConfigType    channels[8];    // Mảng cấu hình các kênh con (cỡ tối đa, hoặc con trỏ config)
    Adc_NotificationCbType   NotificationCb; // Callback EOC
    Adc_DmaHalfCompleteCbType DmaHalfCb;     // Callback 50% buffer
    Adc_DmaCompleteCbType    DmaCompleteCb;  // Callback 100% buffer
} Adc_GroupConfigType;

void ADC1_2_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);

#define ADC_NUM_GROUPS  2
extern const Adc_GroupConfigType AdcGroupConfig[ADC_NUM_GROUPS];

#endif /* ADC_CFG_H */
