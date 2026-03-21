/**********************************************************
 * @file    Adc_Types.h
 * @brief   ADC Driver Type Definitions (AUTOSAR-like)
 * @details File này định nghĩa kiểu dữ liệu, mã lỗi, service ID
 *          và cấu trúc cấu hình cho ADC Driver trên STM32F103 (SPL).
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#ifndef ADC_TYPES_H
#define ADC_TYPES_H

#include "Std_Types.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

/**********************************************************
 * Thông tin module
 **********************************************************/
#define ADC_VENDOR_ID            0x1234U
#define ADC_MODULE_ID            0x007BU
#define ADC_INSTANCE_ID          0x00U

#define ADC_SW_MAJOR_VERSION     1U
#define ADC_SW_MINOR_VERSION     1U
#define ADC_SW_PATCH_VERSION     0U

/**********************************************************
 * Service ID theo SWS ADC
 **********************************************************/
#define ADC_INIT_SID                             0x00U
#define ADC_DEINIT_SID                           0x01U
#define ADC_STARTGROUPCONVERSION_SID             0x02U
#define ADC_STOPGROUPCONVERSION_SID              0x03U
#define ADC_READGROUP_SID                        0x04U
#define ADC_ENABLEHARDWARETRIGGER_SID            0x05U
#define ADC_DISABLEHARDWARETRIGGER_SID           0x06U
#define ADC_ENABLEGROUPNOTIFICATION_SID          0x07U
#define ADC_DISABLEGROUPNOTIFICATION_SID         0x08U
#define ADC_GETGROUPSTATUS_SID                   0x09U
#define ADC_GETVERSIONINFO_SID                   0x0AU
#define ADC_GETSTREAMLASTPOINTER_SID             0x0BU
#define ADC_SETUPRESULTBUFFER_SID                0x0CU
#define ADC_SETPOWERSTATE_SID                    0x10U
#define ADC_GETCURRENTPOWERSTATE_SID             0x11U
#define ADC_GETTARGETPOWERSTATE_SID              0x12U
#define ADC_PREPAREPOWERSTATE_SID                0x13U
#define ADC_MAIN_POWERTRANSITIONMANAGER_SID      0x14U

/**********************************************************
 * Development/Runtime Error Code theo SWS ADC
 **********************************************************/
#define ADC_E_UNINIT                     0x0AU
#define ADC_E_BUSY                       0x0BU
#define ADC_E_IDLE                       0x0CU
#define ADC_E_ALREADY_INITIALIZED        0x0DU
#define ADC_E_PARAM_POINTER              0x14U
#define ADC_E_PARAM_GROUP                0x15U
#define ADC_E_WRONG_CONV_MODE            0x16U
#define ADC_E_WRONG_TRIGG_SRC            0x17U
#define ADC_E_NOTIF_CAPABILITY           0x18U
#define ADC_E_BUFFER_UNINIT              0x19U
#define ADC_E_NOT_DISENGAGED             0x1AU
#define ADC_E_POWER_STATE_NOT_SUPPORTED  0x1BU
#define ADC_E_TRANSITION_NOT_POSSIBLE    0x1CU
#define ADC_E_PERIPHERAL_NOT_PREPARED    0x1DU

/**********************************************************
 * Kiểu dữ liệu cơ bản
 **********************************************************/
typedef uint8  Adc_GroupType;
typedef uint8  Adc_ChannelType;
typedef uint16 Adc_ValueGroupType;
typedef uint16 Adc_StreamNumSampleType;

typedef enum
{
    ADC_IDLE = 0x00U,
    ADC_BUSY = 0x01U,
    ADC_COMPLETED = 0x02U,
    ADC_STREAM_COMPLETED = 0x03U
} Adc_StatusType;

typedef enum
{
    ADC_CONV_MODE_ONESHOT = 0x00U,
    ADC_CONV_MODE_CONTINUOUS = 0x01U
} Adc_ConversionModeType;

typedef enum
{
    ADC_TRIGG_SRC_SW = 0x00U,
    ADC_TRIGG_SRC_HW = 0x01U
} Adc_TriggerSourceType;

typedef enum
{
    ADC_ACCESS_MODE_SINGLE = 0x00U,
    ADC_ACCESS_MODE_STREAMING = 0x01U
} Adc_GroupAccessModeType;

typedef enum
{
    ADC_STREAM_BUFFER_LINEAR = 0x00U,
    ADC_STREAM_BUFFER_CIRCULAR = 0x01U
} Adc_StreamBufferModeType;

/**********************************************************
 * Kiểu dữ liệu power-state
 **********************************************************/
typedef uint8 Adc_PowerStateType;

#define ADC_FULL_POWER        ((Adc_PowerStateType)0x00U)
#define ADC_LOW_POWER_STATE   ((Adc_PowerStateType)0x01U)

typedef enum
{
    ADC_SERVICE_ACCEPTED = 0x00U,
    ADC_NOT_INIT = 0x01U,
    ADC_SEQUENCE_ERROR = 0x02U,
    ADC_HW_FAILURE = 0x03U,
    ADC_POWER_STATE_NOT_SUPP = 0x04U,
    ADC_TRANSITION_NOT_POSSIBLE = 0x05U
} Adc_PowerStateRequestResultType;

/**********************************************************
 * Kiểu callback
 **********************************************************/
typedef void (*Adc_NotificationCbType)(void);

/**********************************************************
 * Cấu trúc cấu hình kênh ADC
 **********************************************************/
typedef struct
{
    Adc_ChannelType ChannelId; /* SPL ADC_Channel_x */
    uint8 Rank;                /* Thứ tự rank: 1..16 */
    uint32 SamplingTime;       /* SPL ADC_SampleTime_xCycles5 */
} Adc_ChannelConfigType;

/**********************************************************
 * Cấu trúc cấu hình DMA cho ADC group
 **********************************************************/
typedef struct
{
    DMA_Channel_TypeDef *DmaChannel;
    uint32 Priority;
    uint32 DmaTcFlag;
    uint32 DmaHtFlag;
    uint32 DmaTcItMask;
    uint32 DmaHtItMask;
} Adc_DmaConfigType;

/**********************************************************
 * Cấu trúc cấu hình group ADC
 **********************************************************/
typedef struct
{
    Adc_GroupType GroupId;
    ADC_TypeDef *AdcInstance;
    const Adc_ChannelConfigType *ChannelList;
    uint8 NumChannels;
    Adc_ConversionModeType ConversionMode;
    Adc_TriggerSourceType TriggerSource;
    uint32 HwTriggerSource;
    Adc_GroupAccessModeType AccessMode;
    Adc_StreamBufferModeType StreamBufferMode;
    Adc_StreamNumSampleType StreamNumSamples;
    const Adc_DmaConfigType *DmaConfig;
    Adc_NotificationCbType NotificationCb;
    Adc_NotificationCbType DmaHalfCb;
    Adc_NotificationCbType DmaCompleteCb;
} Adc_GroupConfigType;

/**********************************************************
 * Cấu trúc cấu hình tổng ADC driver
 **********************************************************/
typedef struct
{
    const Adc_GroupConfigType *Groups;
    uint8 NumGroups;
} Adc_ConfigType;

#endif /* ADC_TYPES_H */
