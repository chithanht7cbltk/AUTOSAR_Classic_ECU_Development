/**********************************************************
 * @file    main_dma.c
 * @brief   Ví dụ ADC Group 1 dùng DMA + hardware trigger API
 * @details Ví dụ minh họa API streaming:
 *          - SetupResultBuffer cho group DMA
 *          - EnableHardwareTrigger/DisableHardwareTrigger
 *          - GetStreamLastPointer để lấy vùng dữ liệu mới nhất
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#include "Adc.h"
#include "Adc_Cfg.h"

volatile Adc_ValueGroupType g_group1_stream[8];
volatile Adc_ValueGroupType *g_last_ptr = (Adc_ValueGroupType *)0;
volatile Adc_StreamNumSampleType g_last_count = 0U;
volatile uint32 g_dma_half_count = 0U;
volatile uint32 g_dma_done_count = 0U;

/**********************************************************
 * @brief   Override callback weak DMA half
 **********************************************************/
void Adc_Group1_DmaHalf(void)
{
    g_dma_half_count++;
}

/**********************************************************
 * @brief   Override callback weak DMA complete
 **********************************************************/
void Adc_Group1_DmaComplete(void)
{
    g_dma_done_count++;
}

int main(void)
{
    Adc_ValueGroupType readback[8];

    /* GPIO analog phải được setup ở Port Driver trước bước này */
    Adc_Init(&AdcDriverConfig);

    (void)Adc_SetupResultBuffer(1U, (Adc_ValueGroupType *)g_group1_stream);
    Adc_EnableHardwareTrigger(1U);

    while (1)
    {
        Adc_StartGroupConversion(1U);
        (void)Adc_ReadGroup(1U, readback);
        g_last_count = Adc_GetStreamLastPointer(1U, (Adc_ValueGroupType **)&g_last_ptr);

        for (volatile uint32 i = 0U; i < 100000U; i++)
        {
            __asm volatile("nop");
        }
    }
}
