/**********************************************************
 * @file    main_combined.c
 * @brief   Ví dụ kết hợp Group SW + Group HW/DMA + power-state API
 * @details Mục tiêu: smoke test toàn bộ API chính của ADC driver.
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"

volatile Adc_ValueGroupType g_group0_buffer[4];
volatile Adc_ValueGroupType g_group1_buffer[8];
volatile Adc_ValueGroupType g_readback0[4];
volatile Adc_ValueGroupType g_readback1[8];
volatile Adc_ValueGroupType *g_last_stream_ptr = (Adc_ValueGroupType *)0;
volatile Adc_StreamNumSampleType g_last_stream_count = 0U;

volatile Adc_PowerStateRequestResultType g_power_result = ADC_SERVICE_ACCEPTED;
volatile Adc_PowerStateType g_power_state = ADC_FULL_POWER;

void Adc_Group0_Notification(void)
{
    /* Hook debug tại đây nếu cần */
}

void Adc_Group1_DmaComplete(void)
{
    /* Hook debug tại đây nếu cần */
}

int main(void)
{
    Std_VersionInfoType versionInfo;

    /* 1) Init ADC + setup buffer */
    Adc_Init(&AdcDriverConfig);
    (void)Adc_SetupResultBuffer(0U, (Adc_ValueGroupType *)g_group0_buffer);
    (void)Adc_SetupResultBuffer(1U, (Adc_ValueGroupType *)g_group1_buffer);

    /* 2) Notification + HW trigger */
    Adc_EnableGroupNotification(0U);
    Adc_EnableHardwareTrigger(1U);

    /* 3) Chạy conversion thử */
    Adc_StartGroupConversion(0U);
    Adc_StartGroupConversion(1U);
    (void)Adc_ReadGroup(0U, (Adc_ValueGroupType *)g_readback0);
    (void)Adc_ReadGroup(1U, (Adc_ValueGroupType *)g_readback1);
    g_last_stream_count = Adc_GetStreamLastPointer(1U, (Adc_ValueGroupType **)&g_last_stream_ptr);

    /* 4) Version + power-state API */
    Adc_GetVersionInfo(&versionInfo);
    (void)Adc_PreparePowerState(ADC_LOW_POWER_STATE, (Adc_PowerStateRequestResultType *)&g_power_result);
    (void)Adc_SetPowerState((Adc_PowerStateRequestResultType *)&g_power_result);
    (void)Adc_GetCurrentPowerState((Adc_PowerStateType *)&g_power_state,
                                   (Adc_PowerStateRequestResultType *)&g_power_result);
    Adc_Main_PowerTransitionManager();

    while (1)
    {
        for (volatile uint32 i = 0U; i < 200000U; i++)
        {
            __asm volatile("nop");
        }
    }
}
