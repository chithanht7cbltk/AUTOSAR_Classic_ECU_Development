/**********************************************************
 * @file    Port.c
 * @brief   Port Driver Source File (AUTOSAR Standard)
 * @details File này hiện thực các API chuẩn AUTOSAR cho Port Driver,
 *          dành cho STM32F103 sử dụng SPL. Driver cho phép cấu hình,
 *          đổi mode, đổi chiều, refresh và đọc thông tin version của port/pin.
 *
 * @version 1.0
 * @date    2024-06-19
 * @author  HALA Academy
 **********************************************************/

/* ===============================
 *           Includes
 * =============================== */
#include "Port.h"
#include "Port_cfg.h"
#include "stm32f10x_rcc.h"
#include <stddef.h>

/* ===============================
 *     Static/Internal Variables
 * =============================== */
static uint8_t Port_Initialized = 0; /* Biến trạng thái xác định Port đã init chưa */

/* ===============================
 *      Internal Helper Function
 * =============================== */

/**********************************************************
 * @brief Cấu hình 1 pin GPIO dựa trên thông số AUTOSAR
 * @param[in] pinCfg Con trỏ đến cấu trúc cấu hình pin
 **********************************************************/
static void Port_ApplyPinConfig(const Port_PinConfigType *pinCfg)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    uint16_t pinMask = PORT_GET_PIN_MASK(pinCfg->PinNum);

    /* Kích hoạt clock cho PORT */
    switch (pinCfg->PortNum)
    {
    case PORT_ID_A:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
        break;
    case PORT_ID_B:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
        break;
    case PORT_ID_C:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
        break;
    case PORT_ID_D:
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
        break;
    default:
        return;
    }

    GPIO_InitStruct.GPIO_Pin = pinMask;
    GPIO_InitStruct.GPIO_Speed = pinCfg->Speed;

    /* Cấu hình mode - chỉ lấy ví dụ mode DIO */
    if (pinCfg->Mode == PORT_PIN_MODE_DIO)
    {
        if (pinCfg->Direction == PORT_PIN_OUT)
        {
            GPIO_InitStruct.GPIO_Mode = (pinCfg->Pull == PORT_PIN_PULL_UP) ? GPIO_Mode_Out_PP : GPIO_Mode_Out_OD;
        }
        else
        {
            if (pinCfg->Pull == PORT_PIN_PULL_UP)
                GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
            else if (pinCfg->Pull == PORT_PIN_PULL_DOWN)
                GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
            else
                GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        }
    }

    if (pinCfg->Mode == PORT_PIN_MODE_ADC)
    {
        if(pinCfg->Direction == PORT_PIN_OUT)
        {
            return
        }
        else
        {
            GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN; // Chế độ analog cho ADC
        }
    }
    /* Các mode khác như ADC, PWM, SPI ... mở rộng thêm tùy MCU */

    /* Khởi tạo chân */
    GPIO_Init(PORT_GET_PORT(pinCfg->PortNum), &GPIO_InitStruct);

    /* Nếu là output, set level ban đầu */
    if (pinCfg->Direction == PORT_PIN_OUT)
    {
        if (pinCfg->Level == PORT_PIN_LEVEL_HIGH)
            GPIO_SetBits(PORT_GET_PORT(pinCfg->PortNum), pinMask);
        else
            GPIO_ResetBits(PORT_GET_PORT(pinCfg->PortNum), pinMask);
    }
}

/* ===============================
 *     Function Definitions
 * =============================== */

/**********************************************************
 * @brief Khởi tạo toàn bộ các Port/Pin theo cấu hình
 * @details Hàm này sẽ gọi cấu hình từng pin theo bảng config.
 * @param[in] ConfigPtr Con trỏ đến cấu hình Port
 **********************************************************/
void Port_Init(const Port_ConfigType *ConfigPtr)
{
    if (ConfigPtr == NULL)
        return;

    for (uint16_t i = 0; i < ConfigPtr->PinCount; i++)
    {
        Port_ApplyPinConfig(&ConfigPtr->PinConfigs[i]);
    }
    Port_Initialized = 1;
}

/**********************************************************
 * @brief Đổi chiều một chân Port (nếu cho phép runtime)
 * @details
 * Hàm sẽ đổi chiều (IN/OUT) của pin, nếu cho phép ở config.
 * @param[in] Pin Số hiệu pin (0..n-1)
 * @param[in] Direction Chiều mong muốn
 **********************************************************/
void Port_SetPinDirection(Port_PinType Pin, Port_PinDirectionType Direction)
{
    if (!Port_Initialized)
        return;
    if (Pin >= PortCfg_PinsCount)
        return;
    if (!PortCfg_Pins[Pin].DirectionChangeable)
        return;

    Port_PinConfigType *pinCfg = (Port_PinConfigType *)&PortCfg_Pins[Pin]; // cast bỏ const để update runtime (nếu cần)
    pinCfg->Direction = Direction;
    Port_ApplyPinConfig(pinCfg);
}

/**********************************************************
 * @brief Làm tươi lại chiều các pin không cho đổi runtime
 * @details Chỉ các pin cấu hình DirectionChangeable=0 sẽ được làm tươi lại chiều về giá trị config
 **********************************************************/
void Port_RefreshPortDirection(void)
{
    if (!Port_Initialized)
        return;
    for (uint16_t i = 0; i < PortCfg_PinsCount; i++)
    {
        if (!PortCfg_Pins[i].DirectionChangeable)
        {
            Port_ApplyPinConfig(&PortCfg_Pins[i]);
        }
    }
}

/**********************************************************
 * @brief Lấy thông tin phiên bản của Port Driver
 * @param[out] versioninfo Con trỏ đến Std_VersionInfoType để nhận version
 **********************************************************/
void Port_GetVersionInfo(Std_VersionInfoType *versioninfo)
{
    if (versioninfo == NULL)
        return;
    versioninfo->vendorID = PORT_VENDOR_ID;
    versioninfo->moduleID = PORT_MODULE_ID;
    versioninfo->sw_major_version = PORT_SW_MAJOR_VERSION;
    versioninfo->sw_minor_version = PORT_SW_MINOR_VERSION;
    versioninfo->sw_patch_version = PORT_SW_PATCH_VERSION;
}

/**********************************************************
 * @brief Đổi mode chức năng của một chân pin (nếu cho phép runtime)
 * @param[in] Pin Số hiệu pin
 * @param[in] Mode Mode chức năng cần chuyển sang
 **********************************************************/
void Port_SetPinMode(Port_PinType Pin, Port_PinModeType Mode)
{
    if (!Port_Initialized)
        return;
    if (Pin >= PortCfg_PinsCount)
        return;
    if (!PortCfg_Pins[Pin].ModeChangeable)
        return;

    Port_PinConfigType *pinCfg = (Port_PinConfigType *)&PortCfg_Pins[Pin];
    pinCfg->Mode = Mode;
    Port_ApplyPinConfig(pinCfg);
}
