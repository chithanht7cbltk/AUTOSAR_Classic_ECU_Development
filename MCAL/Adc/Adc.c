/**********************************************************
 * @file Adc.c
 * @brief Triển khai các hàm điều khiển ADC.
 * @details File này triển khai các hàm đã khai báo trong Adc.h, bao gồm khởi tạo, chuyển đổi,
 *          đọc kết quả và quản lý trạng thái của mô-đun ADC. Được tối giản hóa
 *          theo pattern Interrupt/Callback và bỏ qua GPIO.
 * @version 1.0
 * @author HALA Academy
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dma.h"
#include "misc.h"
#include <stddef.h>

static uint8_t Adc_IsInitialized = 0;
static Adc_ValueGroupType* Adc_ResultBuffers[ADC_NUM_GROUPS] = {NULL};
static Adc_StatusType Adc_GroupStatuses[ADC_NUM_GROUPS];

void Adc_Init(const Adc_ConfigType* ConfigPtr) {
    if (Adc_IsInitialized) return;
    (void)ConfigPtr;

    uint8_t adc1_used = 0;
    uint8_t adc2_used = 0;

    for (uint8_t i = 0; i < ADC_NUM_GROUPS; i++) {
        if (AdcGroupConfig[i].ADCx == ADC1) adc1_used = 1;
        if (AdcGroupConfig[i].ADCx == ADC2) adc2_used = 1;
        Adc_GroupStatuses[i] = ADC_IDLE;
        Adc_ResultBuffers[i] = NULL;
    }

    /* 1. Bật RCC Clock cho ADC dựa theo nhóm được dùng */
    if (adc1_used) RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    if (adc2_used) RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

    /* 2. Cấu hình SPL ADC độc lập và DMA(nếu có) */
    for (uint8_t i = 0; i < ADC_NUM_GROUPS; i++) {
        const Adc_GroupConfigType *groupConfig = &AdcGroupConfig[i];
        
        ADC_InitTypeDef ADC_InitStructure;
        ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
        ADC_InitStructure.ADC_ContinuousConvMode = (groupConfig->triggerType == ADC_TRIGGER_DMA_CIRCULAR) ? ENABLE : DISABLE; 
        ADC_InitStructure.ADC_ScanConvMode = (groupConfig->channelCount > 1) ? ENABLE : DISABLE;
        
        /* Xác định tín hiệu kích phát */
        if (groupConfig->triggerType == ADC_TRIGGER_SOFTWARE || groupConfig->triggerType == ADC_TRIGGER_EOC_INT || groupConfig->triggerType == ADC_TRIGGER_DMA || groupConfig->triggerType == ADC_TRIGGER_DMA_CIRCULAR) {
            ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
        } else {
            ADC_InitStructure.ADC_ExternalTrigConv = (uint32_t)groupConfig->triggerType; // Cast directly HW timers
        }
        
        ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
        ADC_InitStructure.ADC_NbrOfChannel = groupConfig->channelCount;
        
        ADC_Init(groupConfig->ADCx, &ADC_InitStructure);
        
        /* Cấu hình các kênh Rank / SampleTime */
        for(uint8_t c = 0; c < groupConfig->channelCount; c++) {
            ADC_RegularChannelConfig(groupConfig->ADCx, groupConfig->channels[c].channel, groupConfig->channels[c].rank, groupConfig->channels[c].samplingTime);
        }

        /* 3. Cấu hình DMA (nếu có Trigger Mode dính đến DMA) */
        if (groupConfig->triggerType == ADC_TRIGGER_DMA || groupConfig->triggerType == ADC_TRIGGER_DMA_CIRCULAR) {
            if (groupConfig->dmaConfig != NULL) {
                RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
                
                DMA_InitTypeDef DMA_InitStructure;
                DMA_DeInit(groupConfig->dmaConfig->dmaChannel);
                DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(groupConfig->ADCx->DR);
                DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)NULL; // Sẽ thiết lập ở Adc_SetupResultBuffer
                DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
                DMA_InitStructure.DMA_BufferSize = groupConfig->channelCount;
                DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
                DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
                DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
                DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
                DMA_InitStructure.DMA_Mode = (groupConfig->triggerType == ADC_TRIGGER_DMA_CIRCULAR) ? DMA_Mode_Circular : DMA_Mode_Normal;
                DMA_InitStructure.DMA_Priority = groupConfig->dmaConfig->priority;
                DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
                
                DMA_Init(groupConfig->dmaConfig->dmaChannel, &DMA_InitStructure);

                /* Bật NVIC ưu tiên cho DMA Channel nếu có Callback */
                if (groupConfig->DmaCompleteCb != NULL || groupConfig->DmaHalfCb != NULL) {
                    NVIC_InitTypeDef NVIC_InitStructure;
                    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn; // Default ví dụ: dùng DMA1 Ch1 cho ADC1
                    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
                    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
                    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
                    NVIC_Init(&NVIC_InitStructure);
                    
                    if (groupConfig->DmaCompleteCb != NULL) DMA_ITConfig(groupConfig->dmaConfig->dmaChannel, DMA_IT_TC, ENABLE);
                    if (groupConfig->DmaHalfCb != NULL) DMA_ITConfig(groupConfig->dmaConfig->dmaChannel, DMA_IT_HT, ENABLE);
                }
                
                ADC_DMACmd(groupConfig->ADCx, ENABLE);
            }
        }
        
        /* 4. Bật Peripheral */
        ADC_Cmd(groupConfig->ADCx, ENABLE);
        
        /* 5. Hiệu chuẩn ADC */
        ADC_ResetCalibration(groupConfig->ADCx);
        while(ADC_GetResetCalibrationStatus(groupConfig->ADCx));
        ADC_StartCalibration(groupConfig->ADCx);
        while(ADC_GetCalibrationStatus(groupConfig->ADCx));
    }

    Adc_IsInitialized = 1;
}

Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr) {
    if (Group >= ADC_NUM_GROUPS || DataBufferPtr == NULL) return 1; /* E_NOT_OK */
    Adc_ResultBuffers[Group] = DataBufferPtr;
    
    /* Gửi pointer vào DMA memory nếu dùng DMA */
    const Adc_GroupConfigType* cfg = &AdcGroupConfig[Group];
    if ((cfg->triggerType == ADC_TRIGGER_DMA || cfg->triggerType == ADC_TRIGGER_DMA_CIRCULAR) && cfg->dmaConfig != NULL) {
        cfg->dmaConfig->dmaChannel->CMAR = (uint32_t)DataBufferPtr;
    }
    
    return 0; /* E_OK */
}

void Adc_DeInit(void) {
    if (!Adc_IsInitialized) return;
    ADC_DeInit(ADC1);
    ADC_DeInit(ADC2);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_ADC2, DISABLE);
    for (uint8_t i = 0; i < ADC_NUM_GROUPS; i++) {
        Adc_GroupStatuses[i] = ADC_IDLE;
        Adc_ResultBuffers[i] = NULL;
    }
    Adc_IsInitialized = 0;
}

void Adc_StartGroupConversion(Adc_GroupType Group) {
    if (!Adc_IsInitialized || Group >= ADC_NUM_GROUPS) return;
    Adc_GroupStatuses[Group] = ADC_BUSY;
    ADC_SoftwareStartConvCmd(AdcGroupConfig[Group].ADCx, ENABLE);
}

void Adc_StopGroupConversion(Adc_GroupType Group) {
    if (!Adc_IsInitialized || Group >= ADC_NUM_GROUPS) return;
    ADC_SoftwareStartConvCmd(AdcGroupConfig[Group].ADCx, DISABLE);
    Adc_GroupStatuses[Group] = ADC_IDLE;
}

Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr) {
    if (!Adc_IsInitialized || Group >= ADC_NUM_GROUPS || DataBufferPtr == NULL) return 1;
    *DataBufferPtr = ADC_GetConversionValue(AdcGroupConfig[Group].ADCx);
    Adc_GroupStatuses[Group] = ADC_COMPLETED;
    return 0;
}

void Adc_EnableGroupNotification(Adc_GroupType Group) {
    if (!Adc_IsInitialized || Group >= ADC_NUM_GROUPS) return;
    
    ADC_ITConfig(AdcGroupConfig[Group].ADCx, ADC_IT_EOC, ENABLE);
    
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void Adc_DisableGroupNotification(Adc_GroupType Group) {
    if (!Adc_IsInitialized || Group >= ADC_NUM_GROUPS) return;
    ADC_ITConfig(AdcGroupConfig[Group].ADCx, ADC_IT_EOC, DISABLE);
}

void Adc_IsrHandler(ADC_TypeDef* ADCx) {
    for (uint8_t i = 0; i < ADC_NUM_GROUPS; i++) {
        const Adc_GroupConfigType* cfg = &AdcGroupConfig[i];
        if (cfg->ADCx != ADCx) continue;
        
        if (ADC_GetITStatus(ADCx, ADC_IT_EOC)) {
            uint16_t val = ADC_GetConversionValue(ADCx);
            
            /* Lấy kết quả lưu vào buffer */
            if (Adc_ResultBuffers[i] != NULL) {
                Adc_ResultBuffers[i][0] = val; 
            }
            
            Adc_GroupStatuses[i] = ADC_COMPLETED;
            ADC_ClearITPendingBit(ADCx, ADC_IT_EOC);
            
            /* Gọi callback Group */
            if (cfg->NotificationCb) {
                cfg->NotificationCb(); 
            }
        }
    }
}

void Adc_DmaIsrHandler(DMA_Channel_TypeDef* DMAy_Channelx) {
    for (uint8_t i = 0; i < ADC_NUM_GROUPS; i++) {
        const Adc_GroupConfigType* cfg = &AdcGroupConfig[i];
        if (cfg->dmaConfig == NULL || cfg->dmaConfig->dmaChannel != DMAy_Channelx) continue;
        
        /* Check Half Transfer */
        if (DMA_GetITStatus(DMA1_IT_HT1)) { /* FIXME: Sử dụng Macro cờ tương ứng */
            DMA_ClearITPendingBit(DMA1_IT_HT1);
            if (cfg->DmaHalfCb) cfg->DmaHalfCb();
        }
        
        /* Check Transfer Complete */
        if (DMA_GetITStatus(DMA1_IT_TC1)) {
            DMA_ClearITPendingBit(DMA1_IT_TC1);
            Adc_GroupStatuses[i] = ADC_COMPLETED;
            if (cfg->DmaCompleteCb) cfg->DmaCompleteCb();
        }
    }
}

Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group) {
    if (Group >= ADC_NUM_GROUPS) return ADC_IDLE;
    return Adc_GroupStatuses[Group];
}

Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType** PtrToSampleAddress) {
    if (Group >= ADC_NUM_GROUPS || PtrToSampleAddress == NULL || Adc_ResultBuffers[Group] == NULL) {
        return 1;
    }
    *PtrToSampleAddress = Adc_ResultBuffers[Group];
    return 0;
}

void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    if (VersionInfo != NULL) {
        VersionInfo->vendorID = 0x0123;
        VersionInfo->moduleID = 0x0010;
        VersionInfo->sw_major_version = 1;
        VersionInfo->sw_minor_version = 0;
        VersionInfo->sw_patch_version = 0;
    }
}

Std_ReturnType Adc_SetPowerState(Adc_PowerStateType PowerState) {
    if (!Adc_IsInitialized) return 1;
    for (uint8_t i = 0; i < ADC_NUM_GROUPS; i++) {
        if (PowerState == ADC_POWER_STATE_ON) {
            ADC_Cmd(AdcGroupConfig[i].ADCx, ENABLE);
        } else {
            ADC_Cmd(AdcGroupConfig[i].ADCx, DISABLE);
        }
    }
    return 0;
}

Adc_PowerStateType Adc_GetCurrentPowerState(Adc_GroupType Group) {
    if (Group >= ADC_NUM_GROUPS) return ADC_POWER_STATE_OFF;
    if (AdcGroupConfig[Group].ADCx->CR2 & ADC_CR2_ADON) {
        return ADC_POWER_STATE_ON;
    }
    return ADC_POWER_STATE_OFF;
}

Adc_PowerStateType Adc_GetTargetPowerState(Adc_GroupType Group) {
    return Adc_GetCurrentPowerState(Group);
}

Std_ReturnType Adc_PreparePowerState(Adc_GroupType Group) {
    (void)Group;
    return 0;
}
