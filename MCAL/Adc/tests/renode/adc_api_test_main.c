/**********************************************************
 * @file    adc_api_test_main.c
 * @brief   Test harness cho ADC API trên Renode + GDB
 * @details Chương trình chỉ giữ CPU trong vòng lặp.
 *          Toàn bộ testcase sẽ được điều khiển qua lệnh GDB.
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"

/* ===========================================
 *  Biến toàn cục phục vụ quan sát bằng GDB
 * =========================================== */
volatile Std_VersionInfoType g_version_info;

volatile Adc_ValueGroupType g_group0_result_buffer[4];
volatile Adc_ValueGroupType g_group1_stream_buffer[8];
volatile Adc_ValueGroupType g_readback_buffer[8];

volatile Adc_ValueGroupType *g_stream_ptr = (Adc_ValueGroupType *)0;
volatile Adc_StreamNumSampleType g_stream_count = 0U;

volatile Adc_StatusType g_group_status = ADC_IDLE;
volatile Std_ReturnType g_ret = E_NOT_OK;

volatile Adc_PowerStateRequestResultType g_power_result = ADC_SERVICE_ACCEPTED;
volatile Adc_PowerStateType g_power_state = ADC_FULL_POWER;

volatile uint32 g_eoc_irq_count = 0U;
volatile uint32 g_dma_ht_count = 0U;
volatile uint32 g_dma_tc_count = 0U;
volatile uint32 g_test_magic = 0U;

/**********************************************************
 * @brief   Callback notification Group 0
 **********************************************************/
void Adc_Group0_Notification(void)
{
    g_eoc_irq_count++;
}

/**********************************************************
 * @brief   Callback DMA half Group 1
 **********************************************************/
void Adc_Group1_DmaHalf(void)
{
    g_dma_ht_count++;
}

/**********************************************************
 * @brief   Callback DMA complete Group 1
 **********************************************************/
void Adc_Group1_DmaComplete(void)
{
    g_dma_tc_count++;
}

/**********************************************************
 * @brief   Entry point test harness
 **********************************************************/
int main(void)
{
    while (1)
    {
        g_test_magic++;
        __asm volatile("nop");
    }
}
