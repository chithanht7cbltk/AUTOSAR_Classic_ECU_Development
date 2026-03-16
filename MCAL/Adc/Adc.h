/**********************************************************
 * @file Adc.h
 * @brief Khai báo các hàm và kiểu dữ liệu liên quan đến điều khiển ADC
 * @details File này cung cấp các khai báo và định nghĩa cần thiết cho việc khởi tạo,
 *          chuyển đổi, đọc kết quả và quản lý trạng thái của ADC trên vi điều khiển.
 * @version 1.0
 * @author HALA Academy
 **********************************************************/

#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"
#include "stm32f10x.h"
#include "Adc_Types.h"

/* Cấu trúc rỗng do sử dụng extern array AdcGroupConfig[] cho driver (tương tự PWM) */
typedef struct {
    uint8_t dummy;
} Adc_ConfigType;

/* ===============================
 *  Khai báo các hàm API của ADC Driver
 * =============================== */

void Adc_Init(const Adc_ConfigType* ConfigPtr);
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);
void Adc_DeInit(void);
void Adc_StartGroupConversion(Adc_GroupType Group);
void Adc_StopGroupConversion(Adc_GroupType Group);
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType* DataBufferPtr);

void Adc_EnableGroupNotification(Adc_GroupType Group);
void Adc_DisableGroupNotification(Adc_GroupType Group);

Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);
Std_ReturnType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType** PtrToSampleAddress);
void Adc_GetVersionInfo(Std_VersionInfoType* VersionInfo);

Std_ReturnType Adc_SetPowerState(Adc_PowerStateType PowerState);
Adc_PowerStateType Adc_GetCurrentPowerState(Adc_GroupType Group);
Adc_PowerStateType Adc_GetTargetPowerState(Adc_GroupType Group);
Std_ReturnType Adc_PreparePowerState(Adc_GroupType Group);

#endif // ADC_H
