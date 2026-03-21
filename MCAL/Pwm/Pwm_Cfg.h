/**********************************************************
 * @file    Pwm_Cfg.h
 * @brief   PWM Driver Pre-Compile Configuration Header
 * @details Định nghĩa các switch cấu hình pre-compile cho
 *          PWM Driver theo phong cách AUTOSAR.
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#ifndef PWM_CFG_H
#define PWM_CFG_H

#include "Std_Types.h"

/**********************************************************
 * Cấu hình bật/tắt các API tùy chọn
 **********************************************************/
#define PWM_DE_INIT_API               STD_ON
#define PWM_SET_DUTY_CYCLE_API        STD_ON
#define PWM_SET_PERIOD_AND_DUTY_API   STD_ON
#define PWM_SET_OUTPUT_TO_IDLE_API    STD_ON
#define PWM_GET_OUTPUT_STATE_API      STD_ON
#define PWM_VERSION_INFO_API          STD_ON

/**********************************************************
 * Cấu hình notification và power state
 **********************************************************/
#define PWM_NOTIFICATION_SUPPORTED                STD_ON
#define PWM_LOW_POWER_STATES_SUPPORT              STD_OFF
#define PWM_POWER_STATE_ASYNCH_TRANSITION_MODE    STD_OFF

#endif /* PWM_CFG_H */
