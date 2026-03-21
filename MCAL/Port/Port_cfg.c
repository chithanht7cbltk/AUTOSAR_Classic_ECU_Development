/**********************************************************
 * @file    Port_cfg.c
 * @brief   Port Driver Configuration Source File
 * @details File này chứa phần hiện thực các tham số cấu hình 
 *          cho từng chân Port của driver Port theo chuẩn AUTOSAR,
 *          dùng cho STM32F103.
 *
 * @version 1.0
 * @date    2024-06-19
 * @author  HALA Academy & ChatGPT
 **********************************************************/

#include "Port_cfg.h"

/**********************************************************
 * Cấu hình chi tiết cho từng chân GPIO
 * - PortNum:   PORT_ID_A, PORT_ID_B, PORT_ID_C, ...
 * - PinNum:    Số thứ tự chân trong port (0-15)
 * - Mode:      PORT_PIN_MODE_DIO, ...
 * - Direction: PORT_PIN_OUT / PORT_PIN_IN
 * - DirectionChangeable: 1 = Cho phép đổi chiều runtime
 * - Level:     PORT_PIN_LEVEL_HIGH / PORT_PIN_LEVEL_LOW
 * - Pull:      PORT_PIN_PULL_NONE / UP / DOWN
 * - ModeChangeable: 1 = Cho phép đổi mode runtime
 **********************************************************/

const Port_PinConfigType PortCfg_Pins[PortCfg_PinsCount] = {
    /* PA0: DIO, Output, Được phép đổi chiều & mode runtime, mặc định HIGH, không pull */
    {
        .PortNum = PORT_ID_A,
        .PinNum = 0,
        .Mode = PORT_PIN_MODE_DIO,
        .Direction = PORT_PIN_OUT,
        .DirectionChangeable = 1,
        .Level = PORT_PIN_LEVEL_HIGH,
        .Pull = PORT_PIN_PULL_NONE,
        .ModeChangeable = 1
    },
    /* PA1: DIO, Input, Không đổi chiều, pull-up */
    {
        .PortNum = PORT_ID_A,
        .PinNum = 1,
        .Mode = PORT_PIN_MODE_ADC,
        .Direction = PORT_PIN_IN,
        .DirectionChangeable = 0,
        .Level = PORT_PIN_LEVEL_LOW, /* Không ảnh hưởng nếu là input */
        .Pull = PORT_PIN_PULL_UP,
        .ModeChangeable = 0
    },
    /* PB0: DIO, Output, Không đổi chiều, mặc định LOW, không pull */
    {
        .PortNum = PORT_ID_B,
        .PinNum = 0,
        .Mode = PORT_PIN_MODE_DIO,
        .Direction = PORT_PIN_OUT,
        .DirectionChangeable = 0,
        .Level = PORT_PIN_LEVEL_LOW,
        .Pull = PORT_PIN_PULL_NONE,
        .ModeChangeable = 0
    },
    /* PC13: DIO, Input, Được phép đổi mode runtime, pull-down */
    {
        .PortNum = PORT_ID_C,
        .PinNum = 13,
        .Mode = PORT_PIN_MODE_PWM,
        .Direction = PORT_PIN_IN,
        .DirectionChangeable = 1,
        .Level = PORT_PIN_LEVEL_LOW,
        .Pull = PORT_PIN_PULL_DOWN,
        .ModeChangeable = 1
    }
    /* ... Thêm cấu hình cho các chân khác tại đây ... */
};

