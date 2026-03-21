/**********************************************************
 * @file    Pwm.h
 * @brief   Pulse Width Modulation (PWM) Driver Header File
 * @details File này chứa các định nghĩa về kiểu dữ liệu và
 *          khai báo các API của PWM Driver tuân theo chuẩn AUTOSAR.
 *          Driver này được thiết kế để điều khiển Timer PWM của STM32F103
 *          sử dụng thư viện SPL.
 * @version 1.0
 * @date    2024-06-27
 * @author  HALA Academy
 **********************************************************/

#ifndef PWM_H
#define PWM_H

#include "Std_Types.h"     /* Các kiểu dữ liệu chuẩn AUTOSAR */
#include "stm32f10x_tim.h" /* Thư viện SPL: Timer PWM cho STM32F103 */

/**********************************************************
 * Định nghĩa các kiểu dữ liệu của PWM Driver
 **********************************************************/

/**********************************************************
 * @typedef Pwm_ChannelType
 * @brief   Kiểu dữ liệu cho một kênh PWM
 * @details Đây là kiểu định danh cho một kênh PWM cụ thể (0, 1, 2, ...)
 **********************************************************/
typedef uint8 Pwm_ChannelType;

/**********************************************************
 * @typedef Pwm_PeriodType
 * @brief   Kiểu dữ liệu cho chu kỳ của PWM (tính bằng tick)
 **********************************************************/
typedef uint16 Pwm_PeriodType;

/**********************************************************
 * @enum    Pwm_OutputStateType
 * @brief   Trạng thái đầu ra của kênh PWM (HIGH hoặc LOW)
 **********************************************************/
typedef enum
{
    PWM_HIGH = 0x00, /**< Đầu ra mức cao */
    PWM_LOW = 0x01   /**< Đầu ra mức thấp */
} Pwm_OutputStateType;

/**********************************************************
 * @enum    Pwm_EdgeNotificationType
 * @brief   Loại cạnh để thông báo ngắt PWM
 **********************************************************/
typedef enum
{
    PWM_RISING_EDGE = 0x00,  /**< Cạnh lên */
    PWM_FALLING_EDGE = 0x01, /**< Cạnh xuống */
    PWM_BOTH_EDGES = 0x02    /**< Cả hai cạnh */
} Pwm_EdgeNotificationType;

/**********************************************************
 * @enum    Pwm_ChannelClassType
 * @brief   Kiểu kênh PWM (chu kỳ cố định, biến đổi, v.v.)
 **********************************************************/
typedef enum
{
    PWM_VARIABLE_PERIOD = 0x00,     /**< PWM period thay đổi được */
    PWM_FIXED_PERIOD = 0x01,        /**< PWM period cố định */
    PWM_FIXED_PERIOD_SHIFTED = 0x02 /**< PWM period cố định, shifted */
} Pwm_ChannelClassType;

/**********************************************************
 * @enum    Pwm_PowerStateRequestResultType
 * @brief   Kết quả yêu cầu chuyển trạng thái nguồn của PWM
 **********************************************************/
typedef enum
{
    PWM_SERVICE_ACCEPTED = 0x00, /**< Yêu cầu dịch vụ được chấp nhận */
    PWM_NOT_INIT = 0x01,         /**< Module PWM chưa được khởi tạo */
    PWM_SEQUENCE_ERROR = 0x02,   /**< Thứ tự gọi API không hợp lệ */
    PWM_HW_FAILURE = 0x03,       /**< Lỗi phần cứng khi chuyển trạng thái nguồn */
    PWM_POWER_STATE_NOT_SUPP = 0x04, /**< Trạng thái nguồn yêu cầu không được hỗ trợ */
    PWM_TRANS_NOT_POSSIBLE = 0x05     /**< Không thể chuyển trực tiếp giữa hai trạng thái */
} Pwm_PowerStateRequestResultType;

/**********************************************************
 * @typedef Pwm_PowerStateType
 * @brief   Kiểu dữ liệu biểu diễn trạng thái nguồn PWM
 **********************************************************/
typedef uint8 Pwm_PowerStateType;

/**********************************************************
 * @brief   Trạng thái nguồn đầy đủ của PWM
 **********************************************************/
#define PWM_FULL_POWER ((Pwm_PowerStateType)0x00U)

/**********************************************************
 * @struct  Pwm_ChannelConfigType
 * @brief   Cấu trúc cấu hình cho từng kênh PWM
 **********************************************************/
typedef struct
{
    TIM_TypeDef *TIMx;              /**< Timer sử dụng (TIM1, TIM2, ...) */
    uint8 channel;                  /**< Channel số (1, 2, 3, 4) */
    Pwm_ChannelClassType classType; /**< Loại kênh */
    Pwm_PeriodType defaultPeriod;   /**< Chu kỳ mặc định */
    uint16 defaultDutyCycle;        /**< Duty Cycle mặc định (0x0000 - 0x8000) */
    Pwm_OutputStateType polarity;   /**< Đầu ra ban đầu */
    Pwm_OutputStateType idleState;  /**< Trạng thái khi idle */
    void (*NotificationCb)(void);   /**< Callback notification (optional) */
} Pwm_ChannelConfigType;

/**********************************************************
 * @struct  Pwm_ConfigType
 * @brief   Cấu trúc cấu hình tổng thể cho driver PWM
 **********************************************************/
typedef struct
{
    const Pwm_ChannelConfigType *Channels; /**< Danh sách các cấu hình kênh */
    uint8 NumChannels;                     /**< Số lượng kênh PWM */
} Pwm_ConfigType;

/**********************************************************
 * Khai báo các API của PWM Driver (chuẩn AUTOSAR)
 **********************************************************/

/**********************************************************
 * @brief   Khởi tạo PWM driver với cấu hình chỉ định
 * @param   ConfigPtr: Con trỏ tới cấu hình PWM
 **********************************************************/
void Pwm_Init(const Pwm_ConfigType *ConfigPtr);

/**********************************************************
 * @brief   Giải phóng tài nguyên và tắt tất cả kênh PWM
 **********************************************************/
void Pwm_DeInit(void);

/**********************************************************
 * @brief   Cài đặt duty cycle cho kênh PWM
 * @param   ChannelNumber: ID kênh PWM logic (index trong cấu hình)
 * @param   DutyCycle: Tỷ lệ (0x0000 - 0x8000, ứng với 0%-100%)
 **********************************************************/
void Pwm_SetDutyCycle(Pwm_ChannelType ChannelNumber, uint16 DutyCycle);

/**********************************************************
 * @brief   Đặt period và duty cycle cho kênh PWM (nếu hỗ trợ)
 * @param   ChannelNumber: ID kênh PWM logic (index trong cấu hình)
 * @param   Period: Chu kỳ PWM (tính bằng tick timer)
 * @param   DutyCycle: Duty cycle (0x0000 - 0x8000)
 **********************************************************/
void Pwm_SetPeriodAndDuty(Pwm_ChannelType ChannelNumber, Pwm_PeriodType Period, uint16 DutyCycle);

/**********************************************************
 * @brief   Đưa kênh PWM về trạng thái idle
 * @param   ChannelNumber: ID kênh PWM logic (index trong cấu hình)
 **********************************************************/
void Pwm_SetOutputToIdle(Pwm_ChannelType ChannelNumber);

/**********************************************************
 * @brief   Đọc trạng thái đầu ra hiện tại của kênh PWM
 * @param   ChannelNumber: ID kênh PWM logic (index trong cấu hình)
 * @return  PWM_HIGH hoặc PWM_LOW
 **********************************************************/
Pwm_OutputStateType Pwm_GetOutputState(Pwm_ChannelType ChannelNumber);

/**********************************************************
 * @brief   Tắt thông báo ngắt cho kênh PWM
 * @param   ChannelNumber: ID kênh PWM logic (index trong cấu hình)
 **********************************************************/
void Pwm_DisableNotification(Pwm_ChannelType ChannelNumber);

/**********************************************************
 * @brief   Bật thông báo ngắt cạnh lên/xuống/cả 2 cho kênh PWM
 * @param   ChannelNumber: ID kênh PWM logic (index trong cấu hình)
 * @param   Notification:  Loại cạnh cần thông báo
 **********************************************************/
void Pwm_EnableNotification(Pwm_ChannelType ChannelNumber, Pwm_EdgeNotificationType Notification);

/**********************************************************
 * @brief   Lấy thông tin phiên bản của driver PWM
 * @param   versioninfo: Con trỏ tới cấu trúc Std_VersionInfoType để nhận thông tin phiên bản
 **********************************************************/
void Pwm_GetVersionInfo(Std_VersionInfoType *versioninfo);

/**********************************************************
 * @brief   Thiết lập trạng thái nguồn đã chuẩn bị cho module PWM
 * @param   Result: Con trỏ nhận kết quả yêu cầu chuyển trạng thái nguồn
 * @return  E_OK nếu thành công, E_NOT_OK nếu thất bại
 **********************************************************/
Std_ReturnType Pwm_SetPowerState(Pwm_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Lấy trạng thái nguồn hiện tại của module PWM
 * @param   CurrentPowerState: Con trỏ nhận trạng thái nguồn hiện tại
 * @param   Result: Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu đọc thành công, E_NOT_OK nếu thất bại
 **********************************************************/
Std_ReturnType Pwm_GetCurrentPowerState(Pwm_PowerStateType *CurrentPowerState,
                                        Pwm_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Lấy trạng thái nguồn mục tiêu của module PWM
 * @param   TargetPowerState: Con trỏ nhận trạng thái nguồn mục tiêu
 * @param   Result: Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu đọc thành công, E_NOT_OK nếu thất bại
 **********************************************************/
Std_ReturnType Pwm_GetTargetPowerState(Pwm_PowerStateType *TargetPowerState,
                                       Pwm_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Chuẩn bị chuyển module PWM sang trạng thái nguồn yêu cầu
 * @param   PowerState: Trạng thái nguồn mục tiêu
 * @param   Result: Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu chuẩn bị thành công, E_NOT_OK nếu thất bại
 **********************************************************/
Std_ReturnType Pwm_PreparePowerState(Pwm_PowerStateType PowerState,
                                     Pwm_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Hàm nền quản lý tiến trình chuyển trạng thái nguồn PWM
 * @details Hàm này được gọi theo chu kỳ khi bật chế độ chuyển trạng thái nguồn bất đồng bộ.
 **********************************************************/
void Pwm_Main_PowerTransitionManager(void);

#endif /* PWM_H */
