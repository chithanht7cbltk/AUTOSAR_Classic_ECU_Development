/**********************************************************
 * @file    Adc_Cfg.h
 * @brief   ADC Driver Pre-Compile Configuration Header
 * @details Định nghĩa switch cấu hình pre-compile và extern
 *          các đối tượng cấu hình post-build cho ADC Driver.
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#ifndef ADC_CFG_H
#define ADC_CFG_H

#include "Adc.h"

/**********************************************************
 * Switch cấu hình API tùy chọn
 **********************************************************/
#define ADC_DEV_ERROR_DETECT                         STD_ON
#define ADC_DEINIT_API                               STD_ON
#define ADC_ENABLE_START_STOP_GROUP_API              STD_ON
#define ADC_READ_GROUP_API                           STD_ON
#define ADC_HW_TRIGGER_API                           STD_ON
#define ADC_GRP_NOTIF_CAPABILITY                     STD_ON
#define ADC_VERSION_INFO_API                         STD_ON

/**********************************************************
 * Switch cấu hình power-state
 **********************************************************/
#define ADC_LOW_POWER_STATES_SUPPORT                 STD_ON
#define ADC_POWER_STATE_ASYNCH_TRANSITION_MODE       STD_OFF

/**********************************************************
 * Giới hạn cấu hình compile-time
 **********************************************************/
#define ADC_MAX_GROUPS                               2U
#define ADC_MAX_CHANNELS_PER_GROUP                   8U
#define ADC_CONFIGURED_GROUPS                        2U

/**********************************************************
 * Extern cấu hình ADC
 **********************************************************/
extern const Adc_GroupConfigType AdcGroupConfigList[ADC_CONFIGURED_GROUPS];
extern const Adc_ConfigType AdcDriverConfig;

/**********************************************************
 * Vector IRQ cho cấu hình hiện tại
 **********************************************************/
void ADC1_2_IRQHandler(void);
void DMA1_Channel1_IRQHandler(void);

#endif /* ADC_CFG_H */
