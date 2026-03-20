/**********************************************************
 * @file    Port_cfg.h
 * @brief   Port Driver Configuration Header File
 * @details Định nghĩa các macro, struct và extern cho cấu hình chân (Port) AUTOSAR.
 * @version 1.0
 * @date    2024-06-27
 * @author  HALA Academy
 **********************************************************/
#ifndef PORT_CFG_H
#define PORT_CFG_H

#include "Std_Types.h"

/* ==== Định nghĩa macro cho mode, pull, ... ==== */
#define PORT_PIN_MODE_DIO    0
#define PORT_PIN_MODE_AF_PP  1

#define PORT_PIN_OUT         1
#define PORT_PIN_IN          0

#define PORT_ID_A            0
#define PORT_ID_B            1
#define PORT_ID_C            2
#define PORT_ID_D            3

#define PORT_PIN_PULL_NONE   0
#define PORT_PIN_PULL_UP     1
#define PORT_PIN_PULL_DOWN   2

#define PORT_PIN_LEVEL_LOW   0
#define PORT_PIN_LEVEL_HIGH  1


/**********************************************************
 * Số lượng chân Port được cấu hình (tùy chỉnh theo dự án)
 **********************************************************/
#define PortCfg_PinsCount    2U

/**********************************************************
 * Mảng cấu hình chi tiết cho từng chân GPIO
 * (khai báo extern, định nghĩa cụ thể ở port_cfg.c)
 **********************************************************/
extern const Port_PinConfigType PortCfg_Pins[PortCfg_PinsCount];

#endif /* PORT_CFG_H */
 