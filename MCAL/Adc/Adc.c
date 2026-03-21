/**********************************************************
 * @file    Adc.c
 * @brief   ADC Driver Source File
 * @details Hiện thực API ADC chuẩn AUTOSAR cho STM32F103 (SPL).
 *          Driver chỉ cấu hình ngoại vi ADC/DMA/IRQ; không cấu hình GPIO.
 *          GPIO analog cần được cấu hình bởi Port Driver trước khi dùng ADC.
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"
#include "stm32f10x_rcc.h"
#include "misc.h"
#include <stddef.h>

/**********************************************************
 * @brief   Runtime data cho từng ADC group
 * @details Cấu trúc này lưu state nội bộ của mỗi group trong quá trình chạy:
 *          buffer kết quả, trạng thái BUSY/IDLE, số mẫu hợp lệ và cờ trigger/notification.
 **********************************************************/
typedef struct
{
    Adc_ValueGroupType *ResultBufferPtr;
    Adc_ValueGroupType LastValues[ADC_MAX_CHANNELS_PER_GROUP];
    Adc_StreamNumSampleType ValidSamples;
    Adc_StatusType Status;
    uint8 NotificationEnabled;
    uint8 HwTriggerEnabled;
} Adc_GroupRuntimeType;

/**********************************************************
 * @brief   Biến static của driver
 * @details Gom các biến trạng thái toàn cục của module ADC:
 *          con trỏ config hiện tại, trạng thái init, runtime của từng group,
 *          và thông tin power-state.
 **********************************************************/
static const Adc_ConfigType *Adc_CurrentConfigPtr = NULL_PTR;
static Adc_GroupRuntimeType Adc_RuntimeData[ADC_MAX_GROUPS];
static uint8 Adc_IsInitialized = 0U;

static Adc_PowerStateType Adc_CurrentPowerState = ADC_FULL_POWER;
static Adc_PowerStateType Adc_TargetPowerState = ADC_FULL_POWER;
static uint8 Adc_IsPowerStatePrepared = 0U;

/**********************************************************
 * @brief   Helper báo lỗi DET
 * @details Bao gói lời gọi `Det_ReportError` theo compile switch
 *          để toàn bộ API ADC dùng thống nhất một điểm report lỗi.
 * @param[in] ApiId   Service ID của API phát sinh lỗi
 * @param[in] ErrorId Mã lỗi ADC_E_* cần report
 **********************************************************/
static void Adc_ReportDevError(uint8 ApiId, uint8 ErrorId)
{
#if (ADC_DEV_ERROR_DETECT == STD_ON)
    Det_ReportError(ADC_MODULE_ID, ADC_INSTANCE_ID, ApiId, ErrorId);
#endif
    (void)ApiId;
    (void)ErrorId;
}

/**********************************************************
 * @brief   Lấy cấu hình group theo ID logic
 * @details Hàm xác thực cả con trỏ config tổng và biên `Group` trước khi trả cấu hình.
 *          Đây là gate-check dùng lại ở hầu hết public API để giảm lặp mã kiểm tra.
 * @param[in] Group ID group logic
 * @return  Con trỏ tới cấu hình group nếu hợp lệ, ngược lại trả `NULL_PTR`
 **********************************************************/
static const Adc_GroupConfigType *Adc_GetGroupConfig(Adc_GroupType Group)
{
    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return NULL_PTR;
    }

    if (Group >= Adc_CurrentConfigPtr->NumGroups)
    {
        return NULL_PTR;
    }

    return &Adc_CurrentConfigPtr->Groups[Group];
}

/**********************************************************
 * @brief   Reset toàn bộ runtime data về trạng thái mặc định
 * @details Hàm được gọi ở các điểm khởi tạo/de-initialize để xóa dữ liệu cũ,
 *          tránh rò state giữa các vòng đời module.
 **********************************************************/
static void Adc_ResetRuntimeData(void)
{
    uint8 i;
    uint8 j;

    for (i = 0U; i < ADC_MAX_GROUPS; i++)
    {
        Adc_RuntimeData[i].ResultBufferPtr = NULL_PTR;
        Adc_RuntimeData[i].ValidSamples = 0U;
        Adc_RuntimeData[i].Status = ADC_IDLE;
        Adc_RuntimeData[i].NotificationEnabled = 0U;
        Adc_RuntimeData[i].HwTriggerEnabled = 0U;

        for (j = 0U; j < ADC_MAX_CHANNELS_PER_GROUP; j++)
        {
            Adc_RuntimeData[i].LastValues[j] = 0U;
        }
    }
}

/**********************************************************
 * @brief   Mapping ADC instance sang RCC clock bit tương ứng
 * @details Dùng để bật clock APB2 đúng cho ADC1/ADC2 theo instance trong cấu hình group.
 *          Nếu instance không hỗ trợ, hàm trả 0 để caller bỏ qua thao tác bật clock.
 * @param[in] AdcInstance Con trỏ ADC instance (ADC1/ADC2)
 * @return  RCC bit-mask cần dùng cho `RCC_APB2PeriphClockCmd`
 **********************************************************/
static uint32 Adc_GetRccMaskForAdc(const ADC_TypeDef *AdcInstance)
{
    if (AdcInstance == ADC1)
    {
        return RCC_APB2Periph_ADC1;
    }

    if (AdcInstance == ADC2)
    {
        return RCC_APB2Periph_ADC2;
    }

    return 0U;
}

/**********************************************************
 * @brief   Bật NVIC cho ADC IRQ (one-time init)
 * @details Hàm có guard nội bộ để chỉ cấu hình NVIC một lần.
 **********************************************************/
static void Adc_EnsureAdcNvicEnabled(void)
{
    static uint8 nvicConfigured = 0U;

    if (nvicConfigured != 0U)
    {
        return;
    }

    {
        NVIC_InitTypeDef nvicInit;
        nvicInit.NVIC_IRQChannel = ADC1_2_IRQn;
        nvicInit.NVIC_IRQChannelPreemptionPriority = 1U;
        nvicInit.NVIC_IRQChannelSubPriority = 0U;
        nvicInit.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvicInit);
    }

    nvicConfigured = 1U;
}

/**********************************************************
 * @brief   Bật NVIC cho DMA1 Channel1 IRQ (one-time init)
 * @details Hàm có guard nội bộ để chỉ cấu hình NVIC một lần.
 **********************************************************/
static void Adc_EnsureDmaNvicEnabled(void)
{
    static uint8 nvicConfigured = 0U;

    if (nvicConfigured != 0U)
    {
        return;
    }

    {
        NVIC_InitTypeDef nvicInit;
        nvicInit.NVIC_IRQChannel = DMA1_Channel1_IRQn;
        nvicInit.NVIC_IRQChannelPreemptionPriority = 2U;
        nvicInit.NVIC_IRQChannelSubPriority = 0U;
        nvicInit.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvicInit);
    }

    nvicConfigured = 1U;
}

/**********************************************************
 * @brief   Chạy calibration ADC với timeout an toàn
 * @details Trên môi trường mô phỏng/hardware chưa đầy đủ, flag calibration có thể
 *          không chuyển trạng thái như kỳ vọng. Timeout giúp tránh treo vô hạn.
 * @param[in] AdcInstance Con trỏ ADC instance cần calibration
 **********************************************************/
static void Adc_RunCalibrationSafe(ADC_TypeDef *AdcInstance)
{
    uint32 timeout;

    ADC_ResetCalibration(AdcInstance);
    timeout = 200000U;
    while ((ADC_GetResetCalibrationStatus(AdcInstance) != RESET) && (timeout > 0U))
    {
        timeout--;
    }

    ADC_StartCalibration(AdcInstance);
    timeout = 200000U;
    while ((ADC_GetCalibrationStatus(AdcInstance) != RESET) && (timeout > 0U))
    {
        timeout--;
    }
}

/**********************************************************
 * @brief   Cấu hình chuỗi regular channel cho một group
 * @details Hàm lặp theo danh sách channel/rank trong cấu hình group
 *          và ghi vào sequencer của ADC theo thứ tự cấu hình AUTOSAR.
 * @param[in] GroupCfg Con trỏ cấu hình group cần áp dụng
 **********************************************************/
static void Adc_ApplyRegularChannelConfig(const Adc_GroupConfigType *GroupCfg)
{
    uint8 i;

    if ((GroupCfg == NULL_PTR) || (GroupCfg->ChannelList == NULL_PTR))
    {
        return;
    }

    for (i = 0U; i < GroupCfg->NumChannels; i++)
    {
        ADC_RegularChannelConfig(GroupCfg->AdcInstance,
                                 GroupCfg->ChannelList[i].ChannelId,
                                 GroupCfg->ChannelList[i].Rank,
                                 GroupCfg->ChannelList[i].SamplingTime);
    }
}

/**********************************************************
 * @brief   Tính kích thước DMA buffer theo cấu hình group
 * @details Với streaming mode, kích thước được nhân theo `StreamNumSamples`.
 * @param[in] GroupCfg Con trỏ cấu hình group
 * @return  Số phần tử DMA buffer (clamp về 0xFFFF nếu vượt ngưỡng)
 **********************************************************/
static uint16 Adc_ComputeDmaBufferSize(const Adc_GroupConfigType *GroupCfg)
{
    uint32 total;

    if (GroupCfg == NULL_PTR)
    {
        return 1U;
    }

    total = (uint32)GroupCfg->NumChannels;
    if (total == 0U)
    {
        total = 1U;
    }

    if ((GroupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) && (GroupCfg->StreamNumSamples > 1U))
    {
        total *= (uint32)GroupCfg->StreamNumSamples;
    }

    if (total > 0xFFFFU)
    {
        total = 0xFFFFU;
    }

    return (uint16)total;
}

/**********************************************************
 * @brief   Cấu hình DMA cho group đang chạy
 * @details Hàm thiết lập DMA channel theo địa chỉ DR của ADC và buffer runtime của group,
 *          đồng thời cấu hình mode normal/circular, enable IT HT/TC theo callback đã khai báo.
 * @param[in] Group    ID group logic cần cấu hình DMA
 * @param[in] GroupCfg Con trỏ cấu hình group
 **********************************************************/
static void Adc_ConfigureDmaForGroup(Adc_GroupType Group, const Adc_GroupConfigType *GroupCfg)
{
    DMA_InitTypeDef dmaInit;
    uint16 dmaBufferSize;

    if ((GroupCfg == NULL_PTR) || (GroupCfg->DmaConfig == NULL_PTR))
    {
        return;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        return;
    }

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_DeInit(GroupCfg->DmaConfig->DmaChannel);

    dmaBufferSize = Adc_ComputeDmaBufferSize(GroupCfg);

    dmaInit.DMA_PeripheralBaseAddr = (uint32)&(GroupCfg->AdcInstance->DR);
    dmaInit.DMA_MemoryBaseAddr = (uint32)Adc_RuntimeData[Group].ResultBufferPtr;
    dmaInit.DMA_DIR = DMA_DIR_PeripheralSRC;
    dmaInit.DMA_BufferSize = dmaBufferSize;
    dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    dmaInit.DMA_Mode = (GroupCfg->StreamBufferMode == ADC_STREAM_BUFFER_CIRCULAR) ? DMA_Mode_Circular : DMA_Mode_Normal;
    dmaInit.DMA_Priority = GroupCfg->DmaConfig->Priority;
    dmaInit.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(GroupCfg->DmaConfig->DmaChannel, &dmaInit);

    if ((GroupCfg->DmaConfig->DmaTcItMask != 0U) && (GroupCfg->DmaCompleteCb != NULL_PTR))
    {
        DMA_ITConfig(GroupCfg->DmaConfig->DmaChannel, GroupCfg->DmaConfig->DmaTcItMask, ENABLE);
    }

    if ((GroupCfg->DmaConfig->DmaHtItMask != 0U) && (GroupCfg->DmaHalfCb != NULL_PTR))
    {
        DMA_ITConfig(GroupCfg->DmaConfig->DmaChannel, GroupCfg->DmaConfig->DmaHtItMask, ENABLE);
    }

    DMA_Cmd(GroupCfg->DmaConfig->DmaChannel, ENABLE);
    ADC_DMACmd(GroupCfg->AdcInstance, ENABLE);

    Adc_EnsureDmaNvicEnabled();
}

/**********************************************************
 * @brief   Tắt DMA cho group
 * @details Đồng thời disable IT mask và clear pending flag liên quan.
 * @param[in] GroupCfg Con trỏ cấu hình group
 **********************************************************/
static void Adc_DisableDmaForGroup(const Adc_GroupConfigType *GroupCfg)
{
    if ((GroupCfg == NULL_PTR) || (GroupCfg->DmaConfig == NULL_PTR))
    {
        return;
    }

    DMA_Cmd(GroupCfg->DmaConfig->DmaChannel, DISABLE);

    if (GroupCfg->DmaConfig->DmaTcItMask != 0U)
    {
        DMA_ITConfig(GroupCfg->DmaConfig->DmaChannel, GroupCfg->DmaConfig->DmaTcItMask, DISABLE);
    }

    if (GroupCfg->DmaConfig->DmaHtItMask != 0U)
    {
        DMA_ITConfig(GroupCfg->DmaConfig->DmaChannel, GroupCfg->DmaConfig->DmaHtItMask, DISABLE);
    }

    if (GroupCfg->DmaConfig->DmaTcFlag != 0U)
    {
        DMA_ClearITPendingBit(GroupCfg->DmaConfig->DmaTcFlag);
    }

    if (GroupCfg->DmaConfig->DmaHtFlag != 0U)
    {
        DMA_ClearITPendingBit(GroupCfg->DmaConfig->DmaHtFlag);
    }

    ADC_DMACmd(GroupCfg->AdcInstance, DISABLE);
}

/**********************************************************
 * @brief   Áp cấu hình phần cứng ADC theo group
 * @details Hàm này set lại ADC_Init, regular channels, trigger source và DMA
 *          theo đúng group được yêu cầu tại thời điểm start/enable trigger.
 * @param[in] Group           ID group logic cần cấu hình
 * @param[in] EnableHwTrigger 1: bật hardware trigger, 0: dùng software trigger
 **********************************************************/
static void Adc_ConfigureGroupHardware(Adc_GroupType Group, uint8 EnableHwTrigger)
{
    ADC_InitTypeDef adcInit;
    const Adc_GroupConfigType *groupCfg;

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        return;
    }

    adcInit.ADC_Mode = ADC_Mode_Independent;
    adcInit.ADC_ScanConvMode = (groupCfg->NumChannels > 1U) ? ENABLE : DISABLE;
    adcInit.ADC_ContinuousConvMode = (groupCfg->ConversionMode == ADC_CONV_MODE_CONTINUOUS) ? ENABLE : DISABLE;
    adcInit.ADC_ExternalTrigConv = (EnableHwTrigger != 0U) ? groupCfg->HwTriggerSource : ADC_ExternalTrigConv_None;
    adcInit.ADC_DataAlign = ADC_DataAlign_Right;
    adcInit.ADC_NbrOfChannel = groupCfg->NumChannels;

    ADC_Init(groupCfg->AdcInstance, &adcInit);
    Adc_ApplyRegularChannelConfig(groupCfg);
    ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, (EnableHwTrigger != 0U) ? ENABLE : DISABLE);

    if (groupCfg->DmaConfig != NULL_PTR)
    {
        Adc_ConfigureDmaForGroup(Group, groupCfg);
    }
    else
    {
        ADC_DMACmd(groupCfg->AdcInstance, DISABLE);
    }
}

/**********************************************************
 * @brief   Kiểm tra còn notification nào bật trên cùng ADC unit hay không
 * @details Hàm dùng khi tắt notification cho một group để quyết định
 *          có cần disable EOC interrupt trên ADC unit tương ứng hay không.
 * @param[in] AdcInstance ADC unit cần kiểm tra
 * @return  1 nếu còn group bật notification trên unit này, ngược lại trả 0
 **********************************************************/
static uint8 Adc_HasEnabledNotificationOnUnit(const ADC_TypeDef *AdcInstance)
{
    uint8 i;

    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return 0U;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        if ((Adc_CurrentConfigPtr->Groups[i].AdcInstance == AdcInstance) &&
            (Adc_RuntimeData[i].NotificationEnabled != 0U) &&
            (Adc_CurrentConfigPtr->Groups[i].NotificationCb != NULL_PTR))
        {
            return 1U;
        }
    }

    return 0U;
}

/**********************************************************
 * @brief   Áp trạng thái nguồn cho tất cả ADC unit đã cấu hình
 * @details Hàm duyệt danh sách group để gom theo ADC1/ADC2, tránh apply lặp trên cùng unit.
 *          `ADC_FULL_POWER` sẽ enable ADC, còn trạng thái low-power sẽ disable conversion/trigger/ADC.
 * @param[in] PowerState Trạng thái nguồn mục tiêu cần áp
 **********************************************************/
static void Adc_ApplyPowerStateToAllUnits(Adc_PowerStateType PowerState)
{
    uint8 i;
    uint8 adc1Handled;
    uint8 adc2Handled;

    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return;
    }

    adc1Handled = 0U;
    adc2Handled = 0U;

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        ADC_TypeDef *adcInstance = Adc_CurrentConfigPtr->Groups[i].AdcInstance;

        if (adcInstance == ADC1)
        {
            if (adc1Handled != 0U)
            {
                continue;
            }
            adc1Handled = 1U;
        }
        else if (adcInstance == ADC2)
        {
            if (adc2Handled != 0U)
            {
                continue;
            }
            adc2Handled = 1U;
        }
        else
        {
            continue;
        }

        if (PowerState == ADC_FULL_POWER)
        {
            ADC_Cmd(adcInstance, ENABLE);
        }
        else
        {
            ADC_SoftwareStartConvCmd(adcInstance, DISABLE);
            ADC_ExternalTrigConvCmd(adcInstance, DISABLE);
            ADC_Cmd(adcInstance, DISABLE);
        }
    }
}

/**********************************************************
 * @brief   Khởi tạo ADC Driver với bộ cấu hình được chọn
 * @details API này thiết lập baseline cho từng ADC unit:
 *          bật clock ngoại vi, cấu hình ADCCLK, init mặc định, disable trigger/IRQ/DMA,
 *          enable ADC và chạy calibration an toàn.
 *          Hàm không cấu hình GPIO analog; chân phải được Port Driver setup trước.
 * @param[in] ConfigPtr Con trỏ cấu hình tổng thể ADC (mảng group và thông số liên quan)
 **********************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr)
{
    uint8 i;
    uint8 adc1Initialized;
    uint8 adc2Initialized;

    if (Adc_IsInitialized != 0U)
    {
        Adc_ReportDevError(ADC_INIT_SID, ADC_E_ALREADY_INITIALIZED);
        return;
    }

    if ((ConfigPtr == NULL_PTR) || (ConfigPtr->Groups == NULL_PTR) ||
        (ConfigPtr->NumGroups == 0U) || (ConfigPtr->NumGroups > ADC_MAX_GROUPS))
    {
        Adc_ReportDevError(ADC_INIT_SID, ADC_E_PARAM_POINTER);
        return;
    }

    Adc_ResetRuntimeData();
    Adc_CurrentConfigPtr = ConfigPtr;

    /* F1: ADCCLK tối đa 14MHz, thường dùng PCLK2/6 = 12MHz */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    adc1Initialized = 0U;
    adc2Initialized = 0U;

    for (i = 0U; i < ConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &ConfigPtr->Groups[i];
        uint32 rccMask = Adc_GetRccMaskForAdc(groupCfg->AdcInstance);

        if (rccMask != 0U)
        {
            RCC_APB2PeriphClockCmd(rccMask, ENABLE);
        }

        if ((groupCfg->AdcInstance == ADC1) && (adc1Initialized == 0U))
        {
            adc1Initialized = 1U;
        }
        else if ((groupCfg->AdcInstance == ADC2) && (adc2Initialized == 0U))
        {
            adc2Initialized = 1U;
        }
        else
        {
            continue;
        }

        /* Init baseline trên mỗi ADC unit, cấu hình group cụ thể sẽ set lại khi Start/EnableHwTrigger */
        {
            ADC_InitTypeDef adcInit;
            adcInit.ADC_Mode = ADC_Mode_Independent;
            adcInit.ADC_ScanConvMode = DISABLE;
            adcInit.ADC_ContinuousConvMode = DISABLE;
            adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
            adcInit.ADC_DataAlign = ADC_DataAlign_Right;
            adcInit.ADC_NbrOfChannel = 1U;
            ADC_Init(groupCfg->AdcInstance, &adcInit);
        }

        ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, DISABLE);
        ADC_DMACmd(groupCfg->AdcInstance, DISABLE);
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
        ADC_Cmd(groupCfg->AdcInstance, ENABLE);
        Adc_RunCalibrationSafe(groupCfg->AdcInstance);
    }

    for (i = 0U; i < ConfigPtr->NumGroups; i++)
    {
        Adc_RuntimeData[i].Status = ADC_IDLE;
        Adc_RuntimeData[i].HwTriggerEnabled = (ConfigPtr->Groups[i].TriggerSource == ADC_TRIGG_SRC_SW) ? 1U : 0U;
    }

    Adc_CurrentPowerState = ADC_FULL_POWER;
    Adc_TargetPowerState = ADC_FULL_POWER;
    Adc_IsPowerStatePrepared = 0U;
    Adc_IsInitialized = 1U;
}

/**********************************************************
 * @brief   Gán vùng nhớ buffer kết quả cho một ADC group
 * @details Driver lưu con trỏ buffer tại runtime để phục vụ cơ chế đọc kết quả
 *          bằng polling hoặc DMA/ISR callback. API này phải được gọi trước khi start group.
 * @param[in] Group         ID group logic cần gán buffer
 * @param[in] DataBufferPtr Con trỏ vùng dữ liệu nhận mẫu ADC
 * @return  `E_OK` nếu hợp lệ, `E_NOT_OK` nếu module chưa init hoặc tham số sai
 **********************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_SETUPRESULTBUFFER_SID, ADC_E_UNINIT);
        return E_NOT_OK;
    }

    if (Adc_GetGroupConfig(Group) == NULL_PTR)
    {
        Adc_ReportDevError(ADC_SETUPRESULTBUFFER_SID, ADC_E_PARAM_GROUP);
        return E_NOT_OK;
    }

    if (DataBufferPtr == NULL_PTR)
    {
        Adc_ReportDevError(ADC_SETUPRESULTBUFFER_SID, ADC_E_PARAM_POINTER);
        return E_NOT_OK;
    }

    Adc_RuntimeData[Group].ResultBufferPtr = DataBufferPtr;
    Adc_RuntimeData[Group].ValidSamples = 0U;

    return E_OK;
}

/**********************************************************
 * @brief   De-initialize ADC Driver và trả ngoại vi về trạng thái an toàn
 * @details API sẽ dừng conversion đang chạy, disable trigger/IRQ, tắt DMA theo từng group,
 *          sau đó disable ADC unit đã dùng và reset toàn bộ runtime state.
 *          API chỉ biên dịch khi `ADC_DEINIT_API == STD_ON`.
 **********************************************************/
void Adc_DeInit(void)
{
#if (ADC_DEINIT_API == STD_ON)
    uint8 i;
    uint8 adc1Handled;
    uint8 adc2Handled;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_DEINIT_SID, ADC_E_UNINIT);
        return;
    }

    if ((Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        Adc_IsInitialized = 0U;
        return;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];

        ADC_SoftwareStartConvCmd(groupCfg->AdcInstance, DISABLE);
        ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, DISABLE);
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
        ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);

        Adc_DisableDmaForGroup(groupCfg);
    }

    adc1Handled = 0U;
    adc2Handled = 0U;

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];

        if (groupCfg->AdcInstance == ADC1)
        {
            if (adc1Handled != 0U)
            {
                continue;
            }
            adc1Handled = 1U;
        }
        else if (groupCfg->AdcInstance == ADC2)
        {
            if (adc2Handled != 0U)
            {
                continue;
            }
            adc2Handled = 1U;
        }
        else
        {
            continue;
        }

        ADC_Cmd(groupCfg->AdcInstance, DISABLE);
    }

    Adc_CurrentConfigPtr = NULL_PTR;
    Adc_ResetRuntimeData();
    Adc_CurrentPowerState = ADC_FULL_POWER;
    Adc_TargetPowerState = ADC_FULL_POWER;
    Adc_IsPowerStatePrepared = 0U;
    Adc_IsInitialized = 0U;
#else
    /* API bị tắt bởi pre-compile switch */
#endif
}

/**********************************************************
 * @brief   Bắt đầu quá trình chuyển đổi cho một group
 * @details Hàm kiểm tra đầy đủ điều kiện trước khi start:
 *          module đã init, group hợp lệ, đã có result buffer, trạng thái không BUSY
 *          và trigger mode/conversion mode hợp lệ. Sau đó apply cấu hình phần cứng
 *          theo group, cập nhật trạng thái BUSY và kích hoạt SW start nếu cần.
 * @param[in] Group ID group logic cần bắt đầu chuyển đổi
 **********************************************************/
void Adc_StartGroupConversion(Adc_GroupType Group)
{
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_STARTGROUPCONVERSION_SID, ADC_E_UNINIT);
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_STARTGROUPCONVERSION_SID, ADC_E_PARAM_GROUP);
        return;
    }

    if (Adc_RuntimeData[Group].Status == ADC_BUSY)
    {
        Adc_ReportDevError(ADC_STARTGROUPCONVERSION_SID, ADC_E_BUSY);
        return;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        Adc_ReportDevError(ADC_STARTGROUPCONVERSION_SID, ADC_E_BUFFER_UNINIT);
        return;
    }

    if ((groupCfg->TriggerSource == ADC_TRIGG_SRC_HW) && (Adc_RuntimeData[Group].HwTriggerEnabled == 0U))
    {
        Adc_ReportDevError(ADC_STARTGROUPCONVERSION_SID, ADC_E_WRONG_TRIGG_SRC);
        return;
    }

    if ((groupCfg->ConversionMode != ADC_CONV_MODE_ONESHOT) &&
        (groupCfg->ConversionMode != ADC_CONV_MODE_CONTINUOUS))
    {
        Adc_ReportDevError(ADC_STARTGROUPCONVERSION_SID, ADC_E_WRONG_CONV_MODE);
        return;
    }

    Adc_ConfigureGroupHardware(Group, Adc_RuntimeData[Group].HwTriggerEnabled);

    if ((Adc_RuntimeData[Group].NotificationEnabled != 0U) && (groupCfg->NotificationCb != NULL_PTR))
    {
        ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, ENABLE);
        Adc_EnsureAdcNvicEnabled();
    }
    else
    {
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
    }

    Adc_RuntimeData[Group].Status = ADC_BUSY;
    Adc_RuntimeData[Group].ValidSamples = 0U;

    if (groupCfg->TriggerSource == ADC_TRIGG_SRC_SW)
    {
        ADC_ClearFlag(groupCfg->AdcInstance, ADC_FLAG_EOC);
        ADC_SoftwareStartConvCmd(groupCfg->AdcInstance, ENABLE);
    }
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Dừng quá trình chuyển đổi của một group
 * @details Hàm hủy lệnh software start, xóa cờ EOC, disable DMA liên quan
 *          và đưa trạng thái group về `ADC_IDLE`.
 *          API chỉ biên dịch khi `ADC_ENABLE_START_STOP_GROUP_API == STD_ON`.
 * @param[in] Group ID group logic cần dừng chuyển đổi
 **********************************************************/
void Adc_StopGroupConversion(Adc_GroupType Group)
{
#if (ADC_ENABLE_START_STOP_GROUP_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_STOPGROUPCONVERSION_SID, ADC_E_UNINIT);
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_STOPGROUPCONVERSION_SID, ADC_E_PARAM_GROUP);
        return;
    }

    if (Adc_RuntimeData[Group].Status == ADC_IDLE)
    {
        Adc_ReportDevError(ADC_STOPGROUPCONVERSION_SID, ADC_E_IDLE);
        return;
    }

    ADC_SoftwareStartConvCmd(groupCfg->AdcInstance, DISABLE);
    ADC_ClearFlag(groupCfg->AdcInstance, ADC_FLAG_EOC);
    Adc_DisableDmaForGroup(groupCfg);

    Adc_RuntimeData[Group].Status = ADC_IDLE;
    Adc_RuntimeData[Group].ValidSamples = 0U;
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Đọc dữ liệu chuyển đổi của group ra buffer do caller cung cấp
 * @details Với group dùng DMA, dữ liệu được lấy từ runtime buffer đã được DMA cập nhật.
 *          Với group không dùng DMA, hàm đọc trực tiếp từ thanh ghi DR và lưu vào cache.
 *          Sau khi đọc xong, cập nhật `ValidSamples` và trạng thái COMPLETED/STREAM_COMPLETED.
 * @param[in] Group          ID group logic cần đọc
 * @param[out] DataBufferPtr Con trỏ buffer đích nhận dữ liệu kết quả
 * @return  `E_OK` nếu đọc thành công, `E_NOT_OK` nếu precondition không thỏa
 **********************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr)
{
#if (ADC_READ_GROUP_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;
    uint8 i;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_READGROUP_SID, ADC_E_UNINIT);
        return E_NOT_OK;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_READGROUP_SID, ADC_E_PARAM_GROUP);
        return E_NOT_OK;
    }

    if (DataBufferPtr == NULL_PTR)
    {
        Adc_ReportDevError(ADC_READGROUP_SID, ADC_E_PARAM_POINTER);
        return E_NOT_OK;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        Adc_ReportDevError(ADC_READGROUP_SID, ADC_E_BUFFER_UNINIT);
        return E_NOT_OK;
    }

    if (Adc_RuntimeData[Group].Status == ADC_IDLE)
    {
        Adc_ReportDevError(ADC_READGROUP_SID, ADC_E_IDLE);
        return E_NOT_OK;
    }

    if (groupCfg->DmaConfig != NULL_PTR)
    {
        for (i = 0U; (i < groupCfg->NumChannels) && (i < ADC_MAX_CHANNELS_PER_GROUP); i++)
        {
            Adc_RuntimeData[Group].LastValues[i] = Adc_RuntimeData[Group].ResultBufferPtr[i];
        }
    }
    else
    {
        for (i = 0U; (i < groupCfg->NumChannels) && (i < ADC_MAX_CHANNELS_PER_GROUP); i++)
        {
            Adc_ValueGroupType value = (Adc_ValueGroupType)ADC_GetConversionValue(groupCfg->AdcInstance);
            Adc_RuntimeData[Group].LastValues[i] = value;
            Adc_RuntimeData[Group].ResultBufferPtr[i] = value;
        }
    }

    for (i = 0U; (i < groupCfg->NumChannels) && (i < ADC_MAX_CHANNELS_PER_GROUP); i++)
    {
        DataBufferPtr[i] = Adc_RuntimeData[Group].LastValues[i];
    }

    Adc_RuntimeData[Group].ValidSamples = groupCfg->NumChannels;
    Adc_RuntimeData[Group].Status = (groupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) ? ADC_STREAM_COMPLETED : ADC_COMPLETED;

    return E_OK;
#else
    (void)Group;
    (void)DataBufferPtr;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Bật hardware trigger cho group được cấu hình nguồn trigger phần cứng
 * @details Hàm chỉ chấp nhận group có `TriggerSource == ADC_TRIGG_SRC_HW`.
 *          Nếu group đang BUSY thì từ chối để tránh thay đổi trigger giữa phiên đo.
 *          Khi hợp lệ, driver bật cờ runtime và áp lại cấu hình phần cứng cho group.
 * @param[in] Group ID group logic cần bật hardware trigger
 **********************************************************/
void Adc_EnableHardwareTrigger(Adc_GroupType Group)
{
#if (ADC_HW_TRIGGER_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_ENABLEHARDWARETRIGGER_SID, ADC_E_UNINIT);
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_ENABLEHARDWARETRIGGER_SID, ADC_E_PARAM_GROUP);
        return;
    }

    if (groupCfg->TriggerSource != ADC_TRIGG_SRC_HW)
    {
        Adc_ReportDevError(ADC_ENABLEHARDWARETRIGGER_SID, ADC_E_WRONG_TRIGG_SRC);
        return;
    }

    if (Adc_RuntimeData[Group].Status == ADC_BUSY)
    {
        Adc_ReportDevError(ADC_ENABLEHARDWARETRIGGER_SID, ADC_E_NOT_DISENGAGED);
        return;
    }

    Adc_RuntimeData[Group].HwTriggerEnabled = 1U;
    Adc_ConfigureGroupHardware(Group, 1U);
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Tắt hardware trigger cho group
 * @details Chỉ áp dụng cho group có nguồn trigger phần cứng.
 *          API xóa cờ runtime `HwTriggerEnabled` và disable external trigger command.
 * @param[in] Group ID group logic cần tắt hardware trigger
 **********************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType Group)
{
#if (ADC_HW_TRIGGER_API == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_DISABLEHARDWARETRIGGER_SID, ADC_E_UNINIT);
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_DISABLEHARDWARETRIGGER_SID, ADC_E_PARAM_GROUP);
        return;
    }

    if (groupCfg->TriggerSource != ADC_TRIGG_SRC_HW)
    {
        Adc_ReportDevError(ADC_DISABLEHARDWARETRIGGER_SID, ADC_E_WRONG_TRIGG_SRC);
        return;
    }

    Adc_RuntimeData[Group].HwTriggerEnabled = 0U;
    ADC_ExternalTrigConvCmd(groupCfg->AdcInstance, DISABLE);
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Bật notification callback cho group
 * @details Khi bật thành công, driver sẽ enable EOC interrupt cho ADC unit tương ứng
 *          và đảm bảo NVIC đã cấu hình để ISR có thể dispatch callback.
 *          API chỉ biên dịch khi `ADC_GRP_NOTIF_CAPABILITY == STD_ON`.
 * @param[in] Group ID group logic cần bật notification
 **********************************************************/
void Adc_EnableGroupNotification(Adc_GroupType Group)
{
#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_ENABLEGROUPNOTIFICATION_SID, ADC_E_UNINIT);
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_ENABLEGROUPNOTIFICATION_SID, ADC_E_PARAM_GROUP);
        return;
    }

    if (groupCfg->NotificationCb == NULL_PTR)
    {
        Adc_ReportDevError(ADC_ENABLEGROUPNOTIFICATION_SID, ADC_E_NOTIF_CAPABILITY);
        return;
    }

    Adc_RuntimeData[Group].NotificationEnabled = 1U;

    ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);
    ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, ENABLE);
    Adc_EnsureAdcNvicEnabled();
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Tắt notification callback của group
 * @details Driver xóa cờ runtime `NotificationEnabled` của group.
 *          Nếu không còn group nào trên cùng ADC unit bật notification,
 *          EOC interrupt của unit đó sẽ được disable.
 * @param[in] Group ID group logic cần tắt notification
 **********************************************************/
void Adc_DisableGroupNotification(Adc_GroupType Group)
{
#if (ADC_GRP_NOTIF_CAPABILITY == STD_ON)
    const Adc_GroupConfigType *groupCfg;

    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_DISABLEGROUPNOTIFICATION_SID, ADC_E_UNINIT);
        return;
    }

    groupCfg = Adc_GetGroupConfig(Group);
    if (groupCfg == NULL_PTR)
    {
        Adc_ReportDevError(ADC_DISABLEGROUPNOTIFICATION_SID, ADC_E_PARAM_GROUP);
        return;
    }

    Adc_RuntimeData[Group].NotificationEnabled = 0U;

    if (Adc_HasEnabledNotificationOnUnit(groupCfg->AdcInstance) == 0U)
    {
        ADC_ITConfig(groupCfg->AdcInstance, ADC_IT_EOC, DISABLE);
        ADC_ClearITPendingBit(groupCfg->AdcInstance, ADC_IT_EOC);
    }
#else
    (void)Group;
#endif
}

/**********************************************************
 * @brief   Lấy trạng thái runtime hiện tại của group
 * @details Nếu module chưa init hoặc group không hợp lệ, hàm trả về `ADC_IDLE`
 *          như trạng thái an toàn mặc định.
 * @param[in] Group ID group logic cần truy vấn
 * @return  Trạng thái thuộc `Adc_StatusType` của group
 **********************************************************/
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group)
{
    if (Adc_IsInitialized == 0U)
    {
        return ADC_IDLE;
    }

    if (Adc_GetGroupConfig(Group) == NULL_PTR)
    {
        return ADC_IDLE;
    }

    return Adc_RuntimeData[Group].Status;
}

/**********************************************************
 * @brief   Lấy con trỏ buffer mẫu hiện tại và số lượng mẫu hợp lệ
 * @details API trả về con trỏ tới result buffer đã đăng ký bằng `Adc_SetupResultBuffer`.
 *          Thường dùng cho access mode streaming để upper layer lấy dữ liệu gần nhất.
 * @param[in] Group ID group logic cần truy vấn
 * @param[out] PtrToSamplePtr Con trỏ nhận địa chỉ buffer mẫu
 * @return  Số lượng mẫu hợp lệ đang có trong runtime (`ValidSamples`)
 **********************************************************/
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PtrToSamplePtr)
{
    if (Adc_IsInitialized == 0U)
    {
        Adc_ReportDevError(ADC_GETSTREAMLASTPOINTER_SID, ADC_E_UNINIT);
        return 0U;
    }

    if (PtrToSamplePtr == NULL_PTR)
    {
        Adc_ReportDevError(ADC_GETSTREAMLASTPOINTER_SID, ADC_E_PARAM_POINTER);
        return 0U;
    }

    if (Adc_GetGroupConfig(Group) == NULL_PTR)
    {
        Adc_ReportDevError(ADC_GETSTREAMLASTPOINTER_SID, ADC_E_PARAM_GROUP);
        return 0U;
    }

    if (Adc_RuntimeData[Group].ResultBufferPtr == NULL_PTR)
    {
        return 0U;
    }

    *PtrToSamplePtr = Adc_RuntimeData[Group].ResultBufferPtr;
    return Adc_RuntimeData[Group].ValidSamples;
}

/**********************************************************
 * @brief   Trả thông tin phiên bản của ADC Driver
 * @details Điền dữ liệu vendor/module/software version vào cấu trúc `Std_VersionInfoType`.
 *          API chỉ có hiệu lực khi `ADC_VERSION_INFO_API == STD_ON`.
 * @param[out] VersionInfo Con trỏ cấu trúc nhận version info
 **********************************************************/
void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo)
{
#if (ADC_VERSION_INFO_API == STD_ON)
    if (VersionInfo == NULL_PTR)
    {
        Adc_ReportDevError(ADC_GETVERSIONINFO_SID, ADC_E_PARAM_POINTER);
        return;
    }

    VersionInfo->vendorID = ADC_VENDOR_ID;
    VersionInfo->moduleID = ADC_MODULE_ID;
    VersionInfo->sw_major_version = ADC_SW_MAJOR_VERSION;
    VersionInfo->sw_minor_version = ADC_SW_MINOR_VERSION;
    VersionInfo->sw_patch_version = ADC_SW_PATCH_VERSION;
#else
    (void)VersionInfo;
#endif
}

/**********************************************************
 * @brief   Chuẩn bị trạng thái nguồn mục tiêu cho ADC Driver
 * @details API chỉ ghi nhận `Adc_TargetPowerState` và đánh dấu đã chuẩn bị;
 *          việc áp thực tế được thực hiện bởi `Adc_SetPowerState`.
 * @param[in] PowerState Trạng thái nguồn mục tiêu cần chuẩn bị
 * @param[out] Result    Con trỏ nhận kết quả yêu cầu power-state
 * @return  `E_OK` khi chuẩn bị hợp lệ, `E_NOT_OK` khi không hỗ trợ hoặc sai điều kiện
 **********************************************************/
Std_ReturnType Adc_PreparePowerState(Adc_PowerStateType PowerState,
                                     Adc_PowerStateRequestResultType *Result)
{
    if (Result == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    if ((PowerState != ADC_FULL_POWER) && (PowerState != ADC_LOW_POWER_STATE))
    {
        Adc_ReportDevError(ADC_PREPAREPOWERSTATE_SID, ADC_E_POWER_STATE_NOT_SUPPORTED);
        *Result = ADC_POWER_STATE_NOT_SUPP;
        return E_NOT_OK;
    }

    Adc_TargetPowerState = PowerState;
    Adc_IsPowerStatePrepared = 1U;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    (void)PowerState;
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Áp trạng thái nguồn đã chuẩn bị lên các ADC unit
 * @details Hàm kiểm tra chuỗi gọi API (phải Prepare trước), validate trạng thái mục tiêu,
 *          sau đó bật/tắt ngoại vi ADC tương ứng và cập nhật trạng thái nguồn hiện tại.
 * @param[out] Result Con trỏ nhận kết quả yêu cầu power-state
 * @return  `E_OK` nếu áp thành công, `E_NOT_OK` nếu lỗi thứ tự gọi hoặc trạng thái không hợp lệ
 **********************************************************/
Std_ReturnType Adc_SetPowerState(Adc_PowerStateRequestResultType *Result)
{
    if (Result == NULL_PTR)
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    if (Adc_IsPowerStatePrepared == 0U)
    {
        Adc_ReportDevError(ADC_SETPOWERSTATE_SID, ADC_E_PERIPHERAL_NOT_PREPARED);
        *Result = ADC_SEQUENCE_ERROR;
        return E_NOT_OK;
    }

    if ((Adc_TargetPowerState != ADC_FULL_POWER) && (Adc_TargetPowerState != ADC_LOW_POWER_STATE))
    {
        Adc_ReportDevError(ADC_SETPOWERSTATE_SID, ADC_E_TRANSITION_NOT_POSSIBLE);
        *Result = ADC_POWER_STATE_NOT_SUPP;
        return E_NOT_OK;
    }

    Adc_ApplyPowerStateToAllUnits(Adc_TargetPowerState);

    Adc_CurrentPowerState = Adc_TargetPowerState;
    Adc_IsPowerStatePrepared = 0U;

    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Lấy trạng thái nguồn hiện tại của ADC Driver
 * @details Trạng thái trả về là giá trị runtime do driver quản lý sau lần SetPowerState gần nhất.
 * @param[out] CurrentPowerState Con trỏ nhận trạng thái nguồn hiện tại
 * @param[out] Result            Con trỏ nhận kết quả dịch vụ
 * @return  `E_OK` nếu đọc thành công, `E_NOT_OK` nếu tham số sai hoặc module chưa init
 **********************************************************/
Std_ReturnType Adc_GetCurrentPowerState(Adc_PowerStateType *CurrentPowerState,
                                        Adc_PowerStateRequestResultType *Result)
{
    if ((CurrentPowerState == NULL_PTR) || (Result == NULL_PTR))
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    *CurrentPowerState = Adc_CurrentPowerState;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Lấy trạng thái nguồn mục tiêu đang được lưu trong driver
 * @details Giá trị này được đặt bởi `Adc_PreparePowerState` và dùng cho bước apply.
 * @param[out] TargetPowerState Con trỏ nhận trạng thái nguồn mục tiêu
 * @param[out] Result           Con trỏ nhận kết quả dịch vụ
 * @return  `E_OK` nếu đọc thành công, `E_NOT_OK` nếu tham số sai hoặc module chưa init
 **********************************************************/
Std_ReturnType Adc_GetTargetPowerState(Adc_PowerStateType *TargetPowerState,
                                       Adc_PowerStateRequestResultType *Result)
{
    if ((TargetPowerState == NULL_PTR) || (Result == NULL_PTR))
    {
        return E_NOT_OK;
    }

    if (Adc_IsInitialized == 0U)
    {
        *Result = ADC_NOT_INIT;
        return E_NOT_OK;
    }

#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
    *TargetPowerState = Adc_TargetPowerState;
    *Result = ADC_SERVICE_ACCEPTED;
    return E_OK;
#else
    *Result = ADC_POWER_STATE_NOT_SUPP;
    return E_NOT_OK;
#endif
}

/**********************************************************
 * @brief   Hàm nền quản lý chuyển power-state bất đồng bộ
 * @details Khi bật chế độ async (`ADC_POWER_STATE_ASYNCH_TRANSITION_MODE == STD_ON`),
 *          hàm này sẽ kiểm tra cờ prepared và gọi `Adc_SetPowerState` để hoàn tất chuyển trạng thái.
 *          Ở chế độ đồng bộ hoặc không hỗ trợ low-power, hàm là no-op an toàn.
 **********************************************************/
void Adc_Main_PowerTransitionManager(void)
{
#if (ADC_LOW_POWER_STATES_SUPPORT == STD_ON)
#if (ADC_POWER_STATE_ASYNCH_TRANSITION_MODE == STD_ON)
    if ((Adc_IsInitialized != 0U) && (Adc_IsPowerStatePrepared != 0U))
    {
        Adc_PowerStateRequestResultType resultDummy;
        (void)Adc_SetPowerState(&resultDummy);
    }
#else
    /* Chế độ đồng bộ: không cần xử lý nền */
#endif
#else
    /* Không hỗ trợ low-power */
#endif
}

/**********************************************************
 * @brief   Logic ISR cho ngắt EOC của ADCx
 * @details Hàm được gọi bởi vector adapter trong `Adc_Cfg.c`.
 *          ISR sẽ lọc các group thuộc đúng ADC instance, cập nhật giá trị mẫu cuối,
 *          cập nhật trạng thái runtime và gọi notification callback nếu được bật.
 * @param[in] AdcInstance Con trỏ ADC instance phát sinh ngắt
 **********************************************************/
void Adc_IsrHandler(ADC_TypeDef *AdcInstance)
{
    uint8 i;

    if ((Adc_IsInitialized == 0U) || (AdcInstance == NULL_PTR) ||
        (Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return;
    }

    if (ADC_GetITStatus(AdcInstance, ADC_IT_EOC) == RESET)
    {
        return;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];

        if (groupCfg->AdcInstance != AdcInstance)
        {
            continue;
        }

        if (Adc_RuntimeData[i].NotificationEnabled == 0U)
        {
            continue;
        }

        {
            Adc_ValueGroupType value = (Adc_ValueGroupType)ADC_GetConversionValue(AdcInstance);
            Adc_RuntimeData[i].LastValues[0] = value;
            if (Adc_RuntimeData[i].ResultBufferPtr != NULL_PTR)
            {
                Adc_RuntimeData[i].ResultBufferPtr[0] = value;
            }
            Adc_RuntimeData[i].ValidSamples = 1U;
            Adc_RuntimeData[i].Status = ADC_COMPLETED;
        }

        if (groupCfg->NotificationCb != NULL_PTR)
        {
            groupCfg->NotificationCb();
        }
    }

    ADC_ClearITPendingBit(AdcInstance, ADC_IT_EOC);
}

/**********************************************************
 * @brief   Logic ISR cho ngắt DMA channel phục vụ ADC
 * @details Hàm xử lý cả hai sự kiện Half Transfer và Transfer Complete.
 *          Khi TC xảy ra, runtime sẽ cập nhật `LastValues`, `ValidSamples`, trạng thái group
 *          và gọi callback DMA complete nếu có cấu hình.
 * @param[in] DmaChannel Con trỏ DMA channel phát sinh ngắt
 **********************************************************/
void Adc_DmaIsrHandler(DMA_Channel_TypeDef *DmaChannel)
{
    uint8 i;

    if ((Adc_IsInitialized == 0U) || (DmaChannel == NULL_PTR) ||
        (Adc_CurrentConfigPtr == NULL_PTR) || (Adc_CurrentConfigPtr->Groups == NULL_PTR))
    {
        return;
    }

    for (i = 0U; i < Adc_CurrentConfigPtr->NumGroups; i++)
    {
        const Adc_GroupConfigType *groupCfg = &Adc_CurrentConfigPtr->Groups[i];
        uint8 j;

        if ((groupCfg->DmaConfig == NULL_PTR) || (groupCfg->DmaConfig->DmaChannel != DmaChannel))
        {
            continue;
        }

        if ((groupCfg->DmaConfig->DmaHtFlag != 0U) && (DMA_GetITStatus(groupCfg->DmaConfig->DmaHtFlag) != RESET))
        {
            DMA_ClearITPendingBit(groupCfg->DmaConfig->DmaHtFlag);
            if (groupCfg->DmaHalfCb != NULL_PTR)
            {
                groupCfg->DmaHalfCb();
            }
        }

        if ((groupCfg->DmaConfig->DmaTcFlag != 0U) && (DMA_GetITStatus(groupCfg->DmaConfig->DmaTcFlag) != RESET))
        {
            DMA_ClearITPendingBit(groupCfg->DmaConfig->DmaTcFlag);

            if (Adc_RuntimeData[i].ResultBufferPtr != NULL_PTR)
            {
                for (j = 0U; (j < groupCfg->NumChannels) && (j < ADC_MAX_CHANNELS_PER_GROUP); j++)
                {
                    Adc_RuntimeData[i].LastValues[j] = Adc_RuntimeData[i].ResultBufferPtr[j];
                }
            }

            Adc_RuntimeData[i].ValidSamples = groupCfg->NumChannels;
            Adc_RuntimeData[i].Status = (groupCfg->AccessMode == ADC_ACCESS_MODE_STREAMING) ? ADC_STREAM_COMPLETED : ADC_COMPLETED;

            if (groupCfg->DmaCompleteCb != NULL_PTR)
            {
                groupCfg->DmaCompleteCb();
            }
        }
    }
}
