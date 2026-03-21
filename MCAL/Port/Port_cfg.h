/**********************************************************
 * @file    Port_cfg.h
 * @brief   Port Driver Configuration Header File
 * @details File này chứa các định nghĩa cấu hình các chân Port 
 *          cho driver Port theo chuẩn AUTOSAR, dùng trên 
 *          STM32F103 với thư viện SPL.
 *
 * @version 1.0
 * @date    2024-06-19
 * @author  HALA Academy
 **********************************************************/

#ifndef PORT_CFG_H
#define PORT_CFG_H

#include "Port.h"   /* Bao gồm các kiểu dữ liệu chuẩn của Port Driver */

/**********************************************************
 * Số lượng chân Port được cấu hình (tùy chỉnh theo dự án)
 **********************************************************/
#define PortCfg_PinsCount    4U

/**********************************************************
 * Mảng cấu hình chi tiết cho từng chân GPIO
 * (khai báo extern, định nghĩa cụ thể ở port_cfg.c)
 **********************************************************/
extern const Port_PinConfigType PortCfg_Pins[PortCfg_PinsCount];

#endif /* PORT_CFG_H */
