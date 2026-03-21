/**********************************************************
 * @file    main.c
 * @brief   Ứng dụng demo cho AUTOSAR SPI Driver
 * @details
 *  - Khởi tạo Port, Dio, Spi
 *  - Thực hiện truyền dữ liệu qua SPI
 * @version 1.0
 * @date    2024-10-01
 * @author  HALA Academy
 **********************************************************/

#include "Spi.h"
#include "Port.h"
#include "Dio.h"
#include "Spi_Cfg.h"
#include "Std_Types.h"

/* Buffer dữ liệu cho demo */
#define DATA_BUFFER_SIZE 10
Spi_DataBufferType TxBuffer[DATA_BUFFER_SIZE] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A};
Spi_DataBufferType RxBuffer[DATA_BUFFER_SIZE] = {0};

int main(void)
{
    /* =======================
     * 1. Khởi tạo Hệ thống
     * ======================= */
    
    /* Khởi tạo Port Driver để cấu hình các chân GPIO (bao gồm chân SPI) */
    /* Lưu ý: Port_ConfigType và PortCfg_Pins cần được định nghĩa trong Port_Cfg.h/.c */
    /* Ở đây giả định Port_Init đã được cấu hình đúng cho các chân SPI */
    Port_ConfigType portConfig; 
    // Trong thực tế, bạn sẽ truyền cấu hình thực tế vào đây, ví dụ: &Port_Config
    Port_Init(&portConfig);

    /* Khởi tạo Dio Driver (nếu cần dùng CS mềm hoặc các chân GPIO khác) */
    Dio_Init();

    /* Khởi tạo Spi Driver */
    Spi_ConfigType spiConfig;
    /* 
     * Trong AUTOSAR, cấu hình thường được sinh ra (generated) và truyền vào Spi_Init.
     * Hoặc Spi_Init có thể sử dụng cấu hình mặc định nếu tham số là NULL (tùy implementation).
     * Ở đây ta giả lập việc truyền cấu hình.
     */
    Spi_Init(&spiConfig);

    /* =======================
     * 2. Vòng lặp chính
     * ======================= */
    while (1)
    {
        /* 
         * Demo: Ghi dữ liệu vào EEPROM (Sequence 1)
         * Giả sử Sequence 1 (SPI_SEQUENCE_1) chứa Job Write EEPROM (SPI_JOB_WRITE_EEPROM)
         * Job này sử dụng Channel 2 (SPI_CHANNEL_2)
         */

        /* Bước 1: Setup Buffer cho Channel 2 */
        /* Channel 2 được sử dụng trong SPI_JOB_WRITE_EEPROM */
        Std_ReturnType setupStatus = Spi_SetupEB(SPI_CHANNEL_2, TxBuffer, RxBuffer, DATA_BUFFER_SIZE);

        if (setupStatus == E_OK)
        {
            /* Bước 2: Truyền dữ liệu đồng bộ (Sync Transmit) */
            /* Hàm này sẽ block cho đến khi truyền xong */
            Std_ReturnType transmitStatus = Spi_SyncTransmit(SPI_SEQUENCE_1);

            if (transmitStatus == E_OK)
            {
                /* Truyền thành công */
                /* Có thể kiểm tra RxBuffer nếu là lệnh đọc/trao đổi dữ liệu */
            }
            else
            {
                /* Xử lý lỗi truyền */
            }
        }
        else
        {
            /* Xử lý lỗi setup buffer */
        }

        /* 
         * Demo: Đọc trạng thái Sequence
         */
        Spi_SeqResultType seqResult = Spi_GetSequenceResult(SPI_SEQUENCE_1);
        if (seqResult == SPI_SEQ_OK)
        {
            // Sequence đã hoàn thành OK
        }

        /* Delay giả lập hoặc chờ sự kiện tiếp theo */
        for (volatile int i = 0; i < 100000; i++);
    }

    return 0;
}
