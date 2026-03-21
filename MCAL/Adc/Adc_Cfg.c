/**********************************************************
 * @file    Adc_Cfg.c
 * @brief   ADC Driver Configuration Source File
 * @details Cấu hình group ADC cho STM32F103 theo style AUTOSAR.
 *          File này chỉ chứa dữ liệu cấu hình + adapter IRQ.
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Adc_Cfg.h"

/**********************************************************
 * Callback mặc định (weak)
 * - Ứng dụng có thể override bằng hàm cùng tên ở file khác.
 **********************************************************/
void Adc_Group0_Notification(void) __attribute__((weak));
void Adc_Group1_DmaHalf(void) __attribute__((weak));
void Adc_Group1_DmaComplete(void) __attribute__((weak));

void Adc_Group0_Notification(void)
{
    /* Callback mặc định rỗng */
}

void Adc_Group1_DmaHalf(void)
{
    /* Callback mặc định rỗng */
}

void Adc_Group1_DmaComplete(void)
{
    /* Callback mặc định rỗng */
}

/**********************************************************
 * Khai báo cấu hình kênh cho từng group
 **********************************************************/
static const Adc_ChannelConfigType Adc_Group0Channels[] = {
    {
        .ChannelId = ADC_Channel_0,
        .Rank = 1U,
        .SamplingTime = ADC_SampleTime_55Cycles5,
    },
};

static const Adc_ChannelConfigType Adc_Group1Channels[] = {
    {
        .ChannelId = ADC_Channel_0,
        .Rank = 1U,
        .SamplingTime = ADC_SampleTime_55Cycles5,
    },
    {
        .ChannelId = ADC_Channel_1,
        .Rank = 2U,
        .SamplingTime = ADC_SampleTime_55Cycles5,
    },
};

/**********************************************************
 * Cấu hình DMA cho group streaming
 **********************************************************/
static const Adc_DmaConfigType Adc_Group1DmaConfig = {
    .DmaChannel = DMA1_Channel1,
    .Priority = DMA_Priority_High,
    .DmaTcFlag = DMA1_IT_TC1,
    .DmaHtFlag = DMA1_IT_HT1,
    .DmaTcItMask = DMA_IT_TC,
    .DmaHtItMask = DMA_IT_HT,
};

/**********************************************************
 * Danh sách group cấu hình
 * - Group 0: SW trigger, one-shot, single access.
 * - Group 1: HW trigger, continuous, streaming + DMA circular.
 **********************************************************/
const Adc_GroupConfigType AdcGroupConfigList[ADC_CONFIGURED_GROUPS] = {
    {
        .GroupId = 0U,
        .AdcInstance = ADC1,
        .ChannelList = Adc_Group0Channels,
        .NumChannels = (uint8)(sizeof(Adc_Group0Channels) / sizeof(Adc_Group0Channels[0])),
        .ConversionMode = ADC_CONV_MODE_ONESHOT,
        .TriggerSource = ADC_TRIGG_SRC_SW,
        .HwTriggerSource = ADC_ExternalTrigConv_None,
        .AccessMode = ADC_ACCESS_MODE_SINGLE,
        .StreamBufferMode = ADC_STREAM_BUFFER_LINEAR,
        .StreamNumSamples = 1U,
        .DmaConfig = NULL_PTR,
        .NotificationCb = Adc_Group0_Notification,
        .DmaHalfCb = NULL_PTR,
        .DmaCompleteCb = NULL_PTR,
    },
    {
        .GroupId = 1U,
        .AdcInstance = ADC1,
        .ChannelList = Adc_Group1Channels,
        .NumChannels = (uint8)(sizeof(Adc_Group1Channels) / sizeof(Adc_Group1Channels[0])),
        .ConversionMode = ADC_CONV_MODE_CONTINUOUS,
        .TriggerSource = ADC_TRIGG_SRC_HW,
        .HwTriggerSource = ADC_ExternalTrigConv_T2_CC2,
        .AccessMode = ADC_ACCESS_MODE_STREAMING,
        .StreamBufferMode = ADC_STREAM_BUFFER_CIRCULAR,
        .StreamNumSamples = 2U,
        .DmaConfig = &Adc_Group1DmaConfig,
        .NotificationCb = NULL_PTR,
        .DmaHalfCb = Adc_Group1_DmaHalf,
        .DmaCompleteCb = Adc_Group1_DmaComplete,
    },
};

/**********************************************************
 * Cấu hình tổng ADC Driver
 **********************************************************/
const Adc_ConfigType AdcDriverConfig = {
    .Groups = AdcGroupConfigList,
    .NumGroups = ADC_CONFIGURED_GROUPS,
};

/**********************************************************
 * Adapter vector IRQ
 **********************************************************/
void ADC1_2_IRQHandler(void)
{
    Adc_IsrHandler(ADC1);
    Adc_IsrHandler(ADC2);
}

void DMA1_Channel1_IRQHandler(void)
{
    Adc_DmaIsrHandler(DMA1_Channel1);
}
