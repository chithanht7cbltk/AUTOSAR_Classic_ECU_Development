/**********************************************************
 * @file    Pwm.c
 * @brief   Pulse Width Modulation (PWM) Driver Source File
 * @details Hiện thực các API của PWM Driver chuẩn AUTOSAR cho STM32F103, sử dụng SPL.
 *          Quản lý chức năng PWM, không cấu hình chân GPIO.
 *
 * @version 1.0
 * @date    2024-06-27
 * @author  HALA Academy
 **********************************************************/

#include "Pwm.h"
#include "Pwm_Cfg.h" /* File cấu hình PWM (extern) */
#include <stddef.h>

/* ===============================
 *     Static Variables & Defines
 * =============================== */

/* Lưu trữ cấu hình driver PWM hiện tại */
static const Pwm_ConfigType *Pwm_CurrentConfigPtr = NULL;

/* Trạng thái đã khởi tạo của driver PWM */
static uint8 Pwm_IsInitialized = 0;

/* ===============================
 *        Function Definitions
 * =============================== */

/**********************************************************
 * @brief   Khởi tạo PWM driver với cấu hình chỉ định
 * @details Khởi tạo tất cả timer/kênh PWM theo cấu hình. Phần cấu hình chân GPIO phải thực hiện riêng.
 *
 * @param[in] ConfigPtr Con trỏ tới cấu hình PWM
 **********************************************************/
void Pwm_Init(const Pwm_ConfigType *ConfigPtr)
{
    if (Pwm_IsInitialized)
        return;
    if (ConfigPtr == NULL)
        return;

    Pwm_CurrentConfigPtr = ConfigPtr;
    for (uint8 i = 0; i < ConfigPtr->NumChannels; i++)
    {
        const Pwm_ChannelConfigType *channelConfig = &ConfigPtr->Channels[i];

        /* Cấu hình chu kỳ cho timer (ARR) */
        channelConfig->TIMx->ARR = channelConfig->defaultPeriod;

        /* Giá trị compare ban đầu theo duty cycle mặc định */
        uint16_t compareValue = ((uint32_t)channelConfig->defaultPeriod * channelConfig->defaultDutyCycle) >> 15;
        switch (channelConfig->channel)
        {
        case 1:
            channelConfig->TIMx->CCR1 = compareValue;
            break;
        case 2:
            channelConfig->TIMx->CCR2 = compareValue;
            break;
        case 3:
            channelConfig->TIMx->CCR3 = compareValue;
            break;
        case 4:
            channelConfig->TIMx->CCR4 = compareValue;
            break;
        default:
            break;
        }

        /* Bật timer */
        TIM_Cmd(channelConfig->TIMx, ENABLE);

        /* Nếu là TIM1 (advanced), enable main output */
        if (channelConfig->TIMx == TIM1)
        {
            TIM_CtrlPWMOutputs(TIM1, ENABLE);
        }
    }
    Pwm_IsInitialized = 1;
}

/**********************************************************
 * @brief   Dừng tất cả kênh PWM và giải phóng tài nguyên
 **********************************************************/
void Pwm_DeInit(void)
{
    if (!Pwm_IsInitialized)
        return;
    for (uint8 i = 0; i < Pwm_CurrentConfigPtr->NumChannels; i++)
    {
        const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[i];
        TIM_Cmd(channelConfig->TIMx, DISABLE);
        if (channelConfig->TIMx == TIM1)
        {
            TIM_CtrlPWMOutputs(TIM1, DISABLE);
        }
    }
    Pwm_IsInitialized = 0;
}

/**********************************************************
 * @brief   Đặt duty cycle cho một kênh PWM
 * @details Chỉ thay đổi duty cycle, không thay đổi period.
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 * @param[in] DutyCycle     Duty cycle mới (0x0000 - 0x8000)
 **********************************************************/
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle)
{
    if (!Pwm_IsInitialized || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
        return;


    for(int i = 0; i < Pwm_CurrentConfigPtr->NumChannels; i++)
    {
        if(Pwm_CurrentConfigPtr->Channels[i].channel == ChannelNumber)
        {
            Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[i];
            if(Pwm_CurrentConfigPtr->Channels[i].classType == PWM_FIXED_PERIOD)
            {
                return; // Không cho phép thay đổi duty cycle nếu là kênh fixed period
            }
            break;
        }
    }
        
    //const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    uint16_t period = channelConfig->TIMx->ARR;
    uint16_t compareValue = ((uint32_t)period * DutyCycle) >> 15;
    switch (channelConfig->channel)
    {
    case 1:
        channelConfig->TIMx->CCR1 = compareValue;
        break;
    case 2:
        channelConfig->TIMx->CCR2 = compareValue;
        break;
    case 3:
        channelConfig->TIMx->CCR3 = compareValue;
        break;
    case 4:
        channelConfig->TIMx->CCR4 = compareValue;
        break;
    default:
        break;
    }
}

/**********************************************************
 * @brief   Đặt period và duty cycle cho một kênh PWM (nếu hỗ trợ)
 * @details Thay đổi đồng thời period (ARR) và duty cycle (CCR).
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 * @param[in] Period        Chu kỳ PWM mới (tick timer)
 * @param[in] DutyCycle     Duty cycle mới (0x0000 - 0x8000)
 **********************************************************/
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16 DutyCycle)
{
    if (!Pwm_IsInitialized || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
        return;
    const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    if (channelConfig->classType != PWM_VARIABLE_PERIOD)
        return;
    channelConfig->TIMx->ARR = Period;
    uint16_t compareValue = ((uint32_t)Period * DutyCycle) >> 15;
    switch (channelConfig->channel)
    {
    case 1:
        channelConfig->TIMx->CCR1 = compareValue;
        break;
    case 2:
        channelConfig->TIMx->CCR2 = compareValue;
        break;
    case 3:
        channelConfig->TIMx->CCR3 = compareValue;
        break;
    case 4:
        channelConfig->TIMx->CCR4 = compareValue;
        break;
    default:
        break;
    }
}

/**********************************************************
 * @brief   Đưa kênh PWM về trạng thái idle (tắt output)
 **********************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber)
{
    if (!Pwm_IsInitialized || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
        return;
    const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    switch (channelConfig->channel)
    {
    case 1:
        channelConfig->TIMx->CCR1 = 0;
        break;
    case 2:
        channelConfig->TIMx->CCR2 = 0;
        break;
    case 3:
        channelConfig->TIMx->CCR3 = 0;
        break;
    case 4:
        channelConfig->TIMx->CCR4 = 0;
        break;
    default:
        break;
    }
}

/**********************************************************
 * @brief   Đọc trạng thái đầu ra hiện tại của kênh PWM
 * @return  PWM_HIGH nếu compare enable, ngược lại PWM_LOW
 **********************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber)
{
    if (!Pwm_IsInitialized || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
        return PWM_LOW;
    const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    uint16_t isOutputEnabled = 0;
    switch (channelConfig->channel)
    {
    case 1:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC1E);
        break;
    case 2:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC2E);
        break;
    case 3:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC3E);
        break;
    case 4:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC4E);
        break;
    default:
        break;
    }
    return (isOutputEnabled) ? PWM_HIGH : PWM_LOW;
}

/**********************************************************
 * @brief   Tắt thông báo ngắt cho kênh PWM
 * @details Nếu sử dụng callback hoặc ngắt, tắt tại đây.
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 **********************************************************/
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber)
{
    if (!Pwm_IsInitialized || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
        return;
    const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    switch (channelConfig->channel)
    {
    case 1:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC1, DISABLE);
        break;
    case 2:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC2, DISABLE);
        break;
    case 3:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC3, DISABLE);
        break;
    case 4:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC4, DISABLE);
        break;
    default:
        break;
    }
}

/**********************************************************
 * @brief   Bật thông báo ngắt cạnh lên/xuống/cả 2 cho kênh PWM
 * @details Nếu sử dụng callback hoặc ngắt, bật tại đây.
 *
 * @param[in] ChannelNumber Số thứ tự kênh PWM
 * @param[in] Notification  Loại cạnh cần thông báo
 **********************************************************/
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification)
{
    (void)Notification; // Hiện tại chưa phân biệt cạnh, có thể mở rộng nếu dùng input capture
    if (!Pwm_IsInitialized || ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
        return;
    const Pwm_ChannelConfigType *channelConfig = &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
    switch (channelConfig->channel)
    {
    case 1:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC1, ENABLE);
        break;
    case 2:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC2, ENABLE);
        break;
    case 3:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC3, ENABLE);
        break;
    case 4:
        TIM_ITConfig(channelConfig->TIMx, TIM_IT_CC4, ENABLE);
        break;
    default:
        break;
    }
}

/**********************************************************
 * @brief   Lấy thông tin phiên bản của driver PWM
 * @details Trả về thông tin phiên bản module PWM.
 *
 * @param[out] versioninfo Con trỏ tới Std_VersionInfoType để nhận version
 **********************************************************/
void Pwm_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo == NULL)
        return;
    versioninfo->vendorID = 0x1234;
    versioninfo->moduleID = 0xABCD;
    versioninfo->sw_major_version = 1;
    versioninfo->sw_minor_version = 0;
    versioninfo->sw_patch_version = 0;
}
