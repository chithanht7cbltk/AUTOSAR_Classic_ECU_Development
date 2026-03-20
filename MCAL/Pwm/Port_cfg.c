/**********************************************************
 * @file    Port_cfg.c
 * @brief   Port Driver Configuration Source File
 * @details File này chứa cấu hình từng chân (Port) cho hệ thống AUTOSAR,
 *          ví dụ này chỉ cấu hình 2 chân dùng cho PWM trên STM32F103.
 * @version 1.0
 * @date    2024-06-27
 * @author  HALA Academy
 **********************************************************/

#include "Port_cfg.h"

/**********************************************************
 * Cấu hình chi tiết cho các chân sử dụng trong hệ thống
 * - PA0:  PWM output (TIM2_CH1) - Alternate Function Push-Pull
 * - PA7:  PWM output (TIM3_CH2) - Alternate Function Push-Pull
 **********************************************************/
const Port_PinConfigType PortCfg_Pins[] = {
    /* PA0 - TIM2_CH1 - PWM Output */
    {
        .PortNum              = PORT_ID_A,
        .PinNum               = 0,
        .Mode                 = PORT_PIN_MODE_AF_PP,    // Alternate Function Push-Pull
        .Direction            = PORT_PIN_OUT,
        .DirectionChangeable  = 0,
        .Level                = PORT_PIN_LEVEL_LOW,      // Không quan trọng với AF_PP
        .Pull                 = PORT_PIN_PULL_NONE,
        .ModeChangeable       = 0
    },
    /* PA7 - TIM3_CH2 - PWM Output */
    {
        .PortNum              = PORT_ID_A,
        .PinNum               = 7,
        .Mode                 = PORT_PIN_MODE_AF_PP,    // Alternate Function Push-Pull
        .Direction            = PORT_PIN_OUT,
        .DirectionChangeable  = 0,
        .Level                = PORT_PIN_LEVEL_LOW,
        .Pull                 = PORT_PIN_PULL_NONE,
        .ModeChangeable       = 0
    }
};

/* Tổng số chân cấu hình */
const uint32 PortCfg_PinsCount = sizeof(PortCfg_Pins) / sizeof(Port_PinConfigType);
