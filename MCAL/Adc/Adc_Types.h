/**********************************************************
 * @file    Adc_Types.h
 * @brief   ADC Driver Type Definitions (AUTOSAR Standard)
 * @details File này chứa các định nghĩa kiểu dữ liệu cho ADC Driver
 *          tuân theo chuẩn AUTOSAR, dành cho STM32F103.
 * @version 1.0
 * @date    2024-10-03
 * @author  HALA Academy
 **********************************************************/

#ifndef ADC_TYPES_H
#define ADC_TYPES_H

#include "Std_Types.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

/* ===============================
 *  Development Error Codes (AUTOSAR DET)
 * =============================== */
#define ADC_E_UNINIT 0x0A              /**< API called before ADC initialized */
#define ADC_E_ALREADY_INITIALIZED 0x0D /**< ADC already initialized */
#define ADC_E_PARAM_POINTER 0x14       /**< Invalid buffer pointer */
#define ADC_E_PARAM_GROUP 0x15         /**< Invalid group ID */
#define ADC_E_WRONG_CONV_MODE 0x16     /**< Invalid conversion mode */
#define ADC_E_WRONG_TRIGG_SRC 0x17     /**< Invalid trigger source */
#define ADC_E_NOTIF_CAPABILITY 0x18    /**< NULL notification callback */
#define ADC_E_BUFFER_UNINIT 0x19       /**< Buffer not initialized */
#define ADC_E_BUSY 0x0B                /**< Conversion in progress */
#define ADC_E_IDLE 0x0C                /**< Group is idle */

/* ===============================
 *  Version Information
 * =============================== */
#define ADC_SW_MAJOR_VERSION 1U
#define ADC_SW_MINOR_VERSION 0U
#define ADC_SW_PATCH_VERSION 0U
#define ADC_VENDOR_ID 0x0123U
#define ADC_MODULE_ID 0x0010U

/* ===============================
 *  Basic Type Definitions
 * =============================== */
typedef uint8 Adc_GroupType;            /**< Group ID type */
typedef uint16 Adc_ValueGroupType;      /**< ADC conversion result type */
typedef uint8 Adc_ChannelType;          /**< Channel ID type */
typedef uint16 Adc_StreamNumSampleType; /**< Number of samples in streaming mode */

/* ===============================
 *  Status & Result Types
 * =============================== */
typedef enum
{
    ADC_IDLE = 0x00,      /**< ADC is idle */
    ADC_BUSY = 0x01,      /**< Conversion in progress */
    ADC_COMPLETED = 0x02, /**< Conversion completed */
    ADC_ERROR = 0x03      /**< Error occurred */
} Adc_StatusType;

typedef enum
{
    ADC_POWER_STATE_ON = 0x00, /**< ADC power on */
    ADC_POWER_STATE_OFF = 0x01 /**< ADC power off */
} Adc_PowerStateType;

/* ===============================
 *  Configuration Types
 * =============================== */
typedef enum
{
    ADC_CONV_MODE_SINGLE = 0,    /**< Single conversion mode */
    ADC_CONV_MODE_CONTINUOUS = 1 /**< Continuous conversion mode */
} Adc_ConversionModeType;

typedef enum
{
    ADC_TRIGGER_SOFTWARE = 0,     /**< Software trigger */
    ADC_TRIGGER_EOC_INT = 1,      /**< EOC interrupt trigger */
    ADC_TRIGGER_DMA = 2,          /**< DMA trigger */
    ADC_TRIGGER_DMA_CIRCULAR = 3, /**< DMA circular mode */
    ADC_TRIGGER_HW_TIMER = 4      /**< Hardware timer trigger */
} Adc_TriggerType;

typedef enum
{
    ADC_NOTIFICATION_DISABLED = 0, /**< Notification disabled */
    ADC_NOTIFICATION_ENABLED = 1   /**< Notification enabled */
} Adc_NotificationType;

typedef enum
{
    ADC_STREAM_BUFFER_LINEAR = 0x00,  /**< Linear buffer mode */
    ADC_STREAM_BUFFER_CIRCULAR = 0x01 /**< Circular buffer mode */
} Adc_StreamBufferModeType;

typedef enum
{
    ADC_ACCESS_MODE_SINGLE = 0x00,   /**< Single access mode */
    ADC_ACCESS_MODE_STREAMING = 0x01 /**< Streaming access mode */
} Adc_GroupAccessModeType;

/* ===============================
 *  Callback Function Types
 * =============================== */
typedef void (*Adc_NotificationCbType)(void);
typedef void (*Adc_DmaHalfCompleteCbType)(void);
typedef void (*Adc_DmaCompleteCbType)(void);
typedef void (*Adc_InitCallbackType)(void);

/* ===============================
 *  Hardware-Specific Types
 * =============================== */
typedef enum
{
    ADC_INSTANCE_1 = 0, /**< ADC1 instance */
    ADC_INSTANCE_2 = 1  /**< ADC2 instance */
} Adc_InstanceType;

typedef enum
{
    ADC_SAMPLETIME_1CYCLE_5 = 0x0,   /**< 1.5 cycles */
    ADC_SAMPLETIME_7CYCLES_5 = 0x1,  /**< 7.5 cycles */
    ADC_SAMPLETIME_13CYCLES_5 = 0x2, /**< 13.5 cycles */
    ADC_SAMPLETIME_28CYCLES_5 = 0x3, /**< 28.5 cycles */
    ADC_SAMPLETIME_41CYCLES_5 = 0x4, /**< 41.5 cycles */
    ADC_SAMPLETIME_55CYCLES_5 = 0x5, /**< 55.5 cycles */
    ADC_SAMPLETIME_71CYCLES_5 = 0x6, /**< 71.5 cycles */
    ADC_SAMPLETIME_239CYCLES_5 = 0x7 /**< 239.5 cycles */
} Adc_SamplingTimeType;

typedef enum
{
    ADC_RESOLUTION_12BIT = 12 /**< 12-bit resolution */
} Adc_ResolutionType;

/* ===============================
 *  DMA Configuration
 * =============================== */
typedef struct
{
    DMA_Channel_TypeDef *dmaChannel; /**< DMA channel */
    uint32_t priority;               /**< DMA priority */
    uint16_t bufferSize;             /**< Buffer size */
    Adc_StreamBufferModeType mode;   /**< Buffer mode (linear/circular) */
} Adc_DmaConfigType;

/* ===============================
 *  Channel Configuration
 * =============================== */
typedef struct
{
    Adc_ChannelType channel;           /**< Channel number */
    Adc_SamplingTimeType samplingTime; /**< Sampling time */
    uint8_t rank;                      /**< Conversion rank */
} Adc_ChannelConfigType;

/* ===============================
 *  Note: Group Configuration and Driver Configuration 
 *  are defined in Adc_Cfg.h and Adc.h as per User doc
 * =============================== */

#endif /* ADC_TYPES_H */
