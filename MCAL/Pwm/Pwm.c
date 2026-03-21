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

/* Trạng thái nguồn hiện tại/mục tiêu (dùng cho nhóm API power state) */
static Pwm_PowerStateType Pwm_CurrentPowerState = PWM_FULL_POWER;
static Pwm_PowerStateType Pwm_TargetPowerState = PWM_FULL_POWER;
static uint8 Pwm_IsPowerStatePrepared = 0;

/* ===============================
 *          Static Helpers
 * =============================== */

/**********************************************************
 * @brief   Trả về cấu hình kênh PWM theo ID logic
 * @details ID logic được hiểu là index của phần tử trong mảng cấu hình.
 *
 * @param[in] ChannelNumber ID kênh PWM logic
 * @return    Con trỏ tới cấu hình kênh, hoặc NULL nếu không hợp lệ
 **********************************************************/
static const Pwm_ChannelConfigType *Pwm_GetChannelConfig(Pwm_ChannelType ChannelNumber)
{
    if ((Pwm_IsInitialized == 0U) || (Pwm_CurrentConfigPtr == NULL))
    {
        return NULL;
    }

    if (ChannelNumber >= Pwm_CurrentConfigPtr->NumChannels)
    {
        return NULL;
    }

    return &Pwm_CurrentConfigPtr->Channels[ChannelNumber];
}

/**********************************************************
 * @brief   Tính giá trị compare từ period và duty dạng 0x0000..0x8000
 *
 * @param[in] Period    Chu kỳ timer
 * @param[in] DutyCycle Duty cycle AUTOSAR 16-bit
 * @return    Giá trị compare ghi vào CCRx
 **********************************************************/
static uint16 Pwm_ComputeCompareValue(Pwm_PeriodType Period, uint16 DutyCycle)
{
    uint16 duty = DutyCycle;
    if (duty > 0x8000U)
    {
        duty = 0x8000U;
    }

    return (uint16)(((uint32)Period * (uint32)duty) >> 15U);
}

/**********************************************************
 * @brief   Ghi giá trị compare vào CCR tương ứng kênh phần cứng
 *
 * @param[in] ChannelCfg  Con trỏ cấu hình kênh PWM
 * @param[in] CompareValue Giá trị compare cần ghi
 **********************************************************/
static void Pwm_WriteCompareValue(const Pwm_ChannelConfigType *ChannelCfg, uint16 CompareValue)
{
    if (ChannelCfg == NULL)
    {
        return;
    }

    switch (ChannelCfg->channel)
    {
    case 1U:
        ChannelCfg->TIMx->CCR1 = CompareValue;
        break;
    case 2U:
        ChannelCfg->TIMx->CCR2 = CompareValue;
        break;
    case 3U:
        ChannelCfg->TIMx->CCR3 = CompareValue;
        break;
    case 4U:
        ChannelCfg->TIMx->CCR4 = CompareValue;
        break;
    default:
        /* Không làm gì nếu kênh phần cứng không hợp lệ */
        break;
    }
}

/**********************************************************
 * @brief   Bật/tắt interrupt compare cho kênh phần cứng
 *
 * @param[in] ChannelCfg Con trỏ cấu hình kênh PWM
 * @param[in] Enable     ENABLE hoặc DISABLE
 **********************************************************/
static void Pwm_SetCompareInterrupt(const Pwm_ChannelConfigType *ChannelCfg, FunctionalState Enable)
{
    if (ChannelCfg == NULL)
    {
        return;
    }

    switch (ChannelCfg->channel)
    {
    case 1U:
        TIM_ITConfig(ChannelCfg->TIMx, TIM_IT_CC1, Enable);
        break;
    case 2U:
        TIM_ITConfig(ChannelCfg->TIMx, TIM_IT_CC2, Enable);
        break;
    case 3U:
        TIM_ITConfig(ChannelCfg->TIMx, TIM_IT_CC3, Enable);
        break;
    case 4U:
        TIM_ITConfig(ChannelCfg->TIMx, TIM_IT_CC4, Enable);
        break;
    default:
        /* Không làm gì nếu kênh phần cứng không hợp lệ */
        break;
    }
}

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
    if (Pwm_IsInitialized != 0U)
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
        Pwm_WriteCompareValue(channelConfig,
                              Pwm_ComputeCompareValue(channelConfig->defaultPeriod, channelConfig->defaultDutyCycle));

        /* Bật timer */
        TIM_Cmd(channelConfig->TIMx, ENABLE);

        /* Nếu là TIM1 (advanced), enable main output */
        if (channelConfig->TIMx == TIM1)
        {
            TIM_CtrlPWMOutputs(TIM1, ENABLE);
        }
    }
    Pwm_CurrentPowerState = PWM_FULL_POWER;
    Pwm_TargetPowerState = PWM_FULL_POWER;
    Pwm_IsPowerStatePrepared = 0U;
    Pwm_IsInitialized = 1U;
}

/**********************************************************
 * @brief   Dừng tất cả kênh PWM và giải phóng tài nguyên
 **********************************************************/
void Pwm_DeInit(void)
{
    if ((Pwm_IsInitialized == 0U) || (Pwm_CurrentConfigPtr == NULL))
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
    Pwm_CurrentConfigPtr = NULL;
    Pwm_CurrentPowerState = PWM_FULL_POWER;
    Pwm_TargetPowerState = PWM_FULL_POWER;
    Pwm_IsPowerStatePrepared = 0U;
    Pwm_IsInitialized = 0U;
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
    const Pwm_ChannelConfigType *channelConfig = Pwm_GetChannelConfig(ChannelNumber);
    if (channelConfig == NULL)
    {
        return;
    }

    /* AUTOSAR cho phép cập nhật duty cho cả kênh fixed-period và variable-period */
    Pwm_WriteCompareValue(channelConfig, Pwm_ComputeCompareValue((Pwm_PeriodType)channelConfig->TIMx->ARR, DutyCycle));
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
    const Pwm_ChannelConfigType *channelConfig = Pwm_GetChannelConfig(ChannelNumber);
    if (channelConfig == NULL)
        return;

    if (channelConfig->classType != PWM_VARIABLE_PERIOD)
        return;

    channelConfig->TIMx->ARR = Period;
    Pwm_WriteCompareValue(channelConfig, Pwm_ComputeCompareValue(Period, DutyCycle));
}

/**********************************************************
 * @brief   Đưa kênh PWM về trạng thái idle (tắt output)
 **********************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber)
{
    const Pwm_ChannelConfigType *channelConfig = Pwm_GetChannelConfig(ChannelNumber);
    if (channelConfig == NULL)
        return;

    /* Đưa duty về 0% theo cách an toàn, sau đó upper layer có thể kích hoạt lại */
    Pwm_WriteCompareValue(channelConfig, 0U);
}

/**********************************************************
 * @brief   Đọc trạng thái đầu ra hiện tại của kênh PWM
 * @return  PWM_HIGH nếu compare enable, ngược lại PWM_LOW
 **********************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber)
{
    const Pwm_ChannelConfigType *channelConfig = Pwm_GetChannelConfig(ChannelNumber);
    if (channelConfig == NULL)
        return PWM_LOW;

    uint16_t isOutputEnabled = 0U;
    switch (channelConfig->channel)
    {
    case 1U:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC1E);
        break;
    case 2U:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC2E);
        break;
    case 3U:
        isOutputEnabled = (channelConfig->TIMx->CCER & TIM_CCER_CC3E);
        break;
    case 4U:
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
    const Pwm_ChannelConfigType *channelConfig = Pwm_GetChannelConfig(ChannelNumber);
    if (channelConfig == NULL)
        return;

    Pwm_SetCompareInterrupt(channelConfig, DISABLE);
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
    const Pwm_ChannelConfigType *channelConfig = Pwm_GetChannelConfig(ChannelNumber);
    if (channelConfig == NULL)
        return;

    Pwm_SetCompareInterrupt(channelConfig, ENABLE);
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

/**********************************************************
 * @brief   Thiết lập trạng thái nguồn đã chuẩn bị cho module PWM
 * @details Phiên bản baseline hiện tại triển khai đồng bộ, không có chuyển trạng thái nền.
 *
 * @param[out] Result Con trỏ nhận kết quả yêu cầu
 * @return  E_OK nếu chuyển trạng thái thành công, ngược lại E_NOT_OK
 **********************************************************/
Std_ReturnType Pwm_SetPowerState(Pwm_PowerStateRequestResultType *Result)
{
    if (Result == NULL)
    {
        return E_NOT_OK;
    }

    if (Pwm_IsInitialized == 0U)
    {
        *Result = PWM_NOT_INIT;
        return E_NOT_OK;
    }

    if (Pwm_IsPowerStatePrepared == 0U)
    {
        *Result = PWM_SEQUENCE_ERROR;
        return E_NOT_OK;
    }

    Pwm_CurrentPowerState = Pwm_TargetPowerState;
    Pwm_IsPowerStatePrepared = 0U;
    *Result = PWM_SERVICE_ACCEPTED;
    return E_OK;
}

/**********************************************************
 * @brief   Lấy trạng thái nguồn hiện tại của module PWM
 *
 * @param[out] CurrentPowerState Con trỏ nhận trạng thái nguồn hiện tại
 * @param[out] Result            Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu đọc thành công, ngược lại E_NOT_OK
 **********************************************************/
Std_ReturnType Pwm_GetCurrentPowerState(Pwm_PowerStateType *CurrentPowerState,
                                        Pwm_PowerStateRequestResultType *Result)
{
    if ((CurrentPowerState == NULL) || (Result == NULL))
    {
        return E_NOT_OK;
    }

    if (Pwm_IsInitialized == 0U)
    {
        *Result = PWM_NOT_INIT;
        return E_NOT_OK;
    }

    *CurrentPowerState = Pwm_CurrentPowerState;
    *Result = PWM_SERVICE_ACCEPTED;
    return E_OK;
}

/**********************************************************
 * @brief   Lấy trạng thái nguồn mục tiêu của module PWM
 *
 * @param[out] TargetPowerState Con trỏ nhận trạng thái nguồn mục tiêu
 * @param[out] Result           Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu đọc thành công, ngược lại E_NOT_OK
 **********************************************************/
Std_ReturnType Pwm_GetTargetPowerState(Pwm_PowerStateType *TargetPowerState,
                                       Pwm_PowerStateRequestResultType *Result)
{
    if ((TargetPowerState == NULL) || (Result == NULL))
    {
        return E_NOT_OK;
    }

    if (Pwm_IsInitialized == 0U)
    {
        *Result = PWM_NOT_INIT;
        return E_NOT_OK;
    }

    *TargetPowerState = Pwm_TargetPowerState;
    *Result = PWM_SERVICE_ACCEPTED;
    return E_OK;
}

/**********************************************************
 * @brief   Chuẩn bị chuyển module PWM sang trạng thái nguồn yêu cầu
 * @details Bản baseline đánh dấu trạng thái đã chuẩn bị, chưa thực hiện xử lý nền.
 *
 * @param[in]  PowerState Trạng thái nguồn mục tiêu
 * @param[out] Result     Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu chuẩn bị thành công, ngược lại E_NOT_OK
 **********************************************************/
Std_ReturnType Pwm_PreparePowerState(Pwm_PowerStateType PowerState,
                                     Pwm_PowerStateRequestResultType *Result)
{
    if (Result == NULL)
    {
        return E_NOT_OK;
    }

    if (Pwm_IsInitialized == 0U)
    {
        *Result = PWM_NOT_INIT;
        return E_NOT_OK;
    }

    Pwm_TargetPowerState = PowerState;
    Pwm_IsPowerStatePrepared = 1U;
    *Result = PWM_SERVICE_ACCEPTED;
    return E_OK;
}

/**********************************************************
 * @brief   Hàm nền quản lý chuyển trạng thái nguồn PWM
 * @details Phiên bản baseline chưa có chuyển trạng thái bất đồng bộ, giữ hàm rỗng an toàn.
 **********************************************************/
void Pwm_Main_PowerTransitionManager(void)
{
    if (Pwm_IsInitialized == 0U)
    {
        return;
    }

    /* Chưa xử lý bất đồng bộ trong baseline hiện tại */
}
