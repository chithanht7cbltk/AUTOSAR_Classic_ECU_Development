/**********************************************************
 * @file Spi_Hw.h
 * @brief Định nghĩa các hàm điều khiển phần cứng cho giao tiếp SPI
 * @details File này chứa các định nghĩa hàm và macro để điều khiển
 *          và quản lý phần cứng SPI trên vi điều khiển STM32. Bao gồm
 *          các hàm khởi tạo, bật/tắt, truyền nhận dữ liệu, và quản lý 
 *          trạng thái của chân NSS cho các kênh SPI1 và SPI2.
 * @version 1.0
 * @date 2024-10-01
 * @author HALA Academy
 **********************************************************/

#ifndef SPI_HW_H
#define SPI_HW_H

#include "stm32f10x_spi.h"   // Thư viện SPI của STM32
#include "stm32f10x_gpio.h"  // Thư viện GPIO của STM32
#include "stm32f10x_rcc.h"   // Thư viện xung nhịp của STM32

/**********************************************************
 * @brief Định nghĩa trạng thái của NSS (Slave Select)
 **********************************************************/
typedef enum {
    SPI_NSS_LOW = 0,    /**< Kéo chân NSS xuống thấp (kích hoạt chọn Slave) */
    SPI_NSS_HIGH = 1    /**< Kéo chân NSS lên cao (giải phóng chọn Slave) */
} Spi_NssStateType;

/**********************************************************
 * @section Định nghĩa các giá trị cho cấu hình SPI
 **********************************************************/

// Các giá trị BaudRate Prescaler cho SPI (giảm xung nhịp từ clock nguồn)
#define SPI_BaudRatePrescaler_2      SPI_BaudRatePrescaler_2    /**< Tốc độ baud là clock nguồn / 2 */
#define SPI_BaudRatePrescaler_4      SPI_BaudRatePrescaler_4    /**< Tốc độ baud là clock nguồn / 4 */
#define SPI_BaudRatePrescaler_8      SPI_BaudRatePrescaler_8    /**< Tốc độ baud là clock nguồn / 8 */
#define SPI_BaudRatePrescaler_16     SPI_BaudRatePrescaler_16   /**< Tốc độ baud là clock nguồn / 16 */
#define SPI_BaudRatePrescaler_32     SPI_BaudRatePrescaler_32   /**< Tốc độ baud là clock nguồn / 32 */
#define SPI_BaudRatePrescaler_64     SPI_BaudRatePrescaler_64   /**< Tốc độ baud là clock nguồn / 64 */
#define SPI_BaudRatePrescaler_128    SPI_BaudRatePrescaler_128  /**< Tốc độ baud là clock nguồn / 128 */
#define SPI_BaudRatePrescaler_256    SPI_BaudRatePrescaler_256  /**< Tốc độ baud là clock nguồn / 256 */

// Các giá trị CPOL (Clock Polarity - Cực tính xung nhịp)
#define SPI_CPOL_Low     SPI_CPOL_Low    /**< Xung nhịp thấp khi nhàn rỗi */
#define SPI_CPOL_High    SPI_CPOL_High   /**< Xung nhịp cao khi nhàn rỗi */

// Các giá trị CPHA (Clock Phase - Pha xung nhịp)
#define SPI_CPHA_1Edge   SPI_CPHA_1Edge  /**< Lấy mẫu dữ liệu ở cạnh đầu tiên */
#define SPI_CPHA_2Edge   SPI_CPHA_2Edge  /**< Lấy mẫu dữ liệu ở cạnh thứ hai */

// Các giá trị Mode (Master hoặc Slave)
#define SPI_MODE_MASTER  SPI_Mode_Master  /**< Chế độ Master (Thiết bị điều khiển) */
#define SPI_MODE_SLAVE   SPI_Mode_Slave   /**< Chế độ Slave (Thiết bị bị điều khiển) */

// Các giá trị quản lý NSS (Slave Select)
#define SPI_NSS_Soft     SPI_NSS_Soft     /**< Quản lý NSS bằng phần mềm */
#define SPI_NSS_Hard     SPI_NSS_Hard     /**< Quản lý NSS bằng phần cứng */

// Các giá trị kích thước dữ liệu
#define SPI_DATASIZE_8BIT   SPI_DataSize_8b   /**< Truyền dữ liệu 8-bit */
#define SPI_DATASIZE_16BIT  SPI_DataSize_16b  /**< Truyền dữ liệu 16-bit */

/**********************************************************
 * @brief Bổ sung các định nghĩa về xung nhịp cho SPI1 và SPI2
 **********************************************************/
#define SPI1_CLOCK_RCC   RCC_APB2Periph_SPI1   /**< Xung nhịp cho SPI1 */
#define SPI2_CLOCK_RCC   RCC_APB1Periph_SPI2   /**< Xung nhịp cho SPI2 */

/**********************************************************
 * @section Định nghĩa cổng và chân GPIO cho SPI1
 **********************************************************/
#define SPI1_GPIO_RCC    RCC_APB2Periph_GPIOA  /**< Xung nhịp cho GPIOA (SPI1) */
#define SPI1_GPIO_PORT   GPIOA                 /**< Cổng GPIOA */
#define SPI1_SCK_PIN     GPIO_Pin_5            /**< Chân SCK của SPI1 (PA5) */
#define SPI1_MISO_PIN    GPIO_Pin_6            /**< Chân MISO của SPI1 (PA6) */
#define SPI1_MOSI_PIN    GPIO_Pin_7            /**< Chân MOSI của SPI1 (PA7) */
#define SPI1_NSS_PIN     GPIO_Pin_4            /**< Chân NSS của SPI1 (PA4) */

/**********************************************************
 * @section Định nghĩa cổng và chân GPIO cho SPI2
 **********************************************************/
#define SPI2_GPIO_RCC    RCC_APB2Periph_GPIOB  /**< Xung nhịp cho GPIOB (SPI2) */
#define SPI2_GPIO_PORT   GPIOB                 /**< Cổng GPIOB */
#define SPI2_SCK_PIN     GPIO_Pin_13           /**< Chân SCK của SPI2 (PB13) */
#define SPI2_MISO_PIN    GPIO_Pin_14           /**< Chân MISO của SPI2 (PB14) */
#define SPI2_MOSI_PIN    GPIO_Pin_15           /**< Chân MOSI của SPI2 (PB15) */
#define SPI2_NSS_PIN     GPIO_Pin_12           /**< Chân NSS của SPI2 (PB12) */

/**********************************************************
 * @brief Macro để bật xung nhịp và cấu hình GPIO cho SPI
 * @param[in] SPI_CHANNEL Kênh SPI (ví dụ: SPI1, SPI2)
 * @param[in] RCC_APB SPI xung nhịp của bộ ngoại vi (ví dụ: RCC_APB2Periph_SPI1)
 * @param[in] RCC_GPIO Xung nhịp cho GPIO (ví dụ: RCC_APB2Periph_GPIOA)
 * @param[in] GPIO_PORT Cổng GPIO (ví dụ: GPIOA)
 * @param[in] GPIO_PINS Chân GPIO (ví dụ: GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7)
 **********************************************************/
#define SETUP_SPI_GPIO(SPI_CHANNEL, RCC_APB, RCC_GPIO, GPIO_PORT, GPIO_PINS)  \
    do {                                                                      \
        /* Bật xung nhịp cho SPI và GPIO */                                   \
        if ((SPI_CHANNEL) == SPI1) {                                          \
            RCC_APB2PeriphClockCmd(RCC_APB, ENABLE);                          \
        } else {                                                              \
            RCC_APB1PeriphClockCmd(RCC_APB, ENABLE);                          \
        }                                                                     \
        RCC_APB2PeriphClockCmd(RCC_GPIO, ENABLE);                             \
        /* Cấu hình GPIO cho SPI */                                           \
        GPIO_InitTypeDef GPIO_InitStruct;                                     \
        GPIO_InitStruct.GPIO_Pin = (GPIO_PINS);                               \
        GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;                          \
        GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;                        \
        GPIO_Init((GPIO_PORT), &GPIO_InitStruct);                             \
    } while(0)

/**********************************************************
 * @brief Khởi tạo phần cứng cho SPI1
 **********************************************************/
static inline void Spi_Hw_Init_SPI1(void) {
    // Bật xung nhịp và cấu hình GPIO cho SPI1
    SETUP_SPI_GPIO(SPI1, SPI1_CLOCK_RCC, SPI1_GPIO_RCC, SPI1_GPIO_PORT, 
                   SPI1_SCK_PIN | SPI1_MISO_PIN | SPI1_MOSI_PIN);
    // Cấu hình chân NSS nếu cần
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = SPI1_NSS_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI1_GPIO_PORT, &GPIO_InitStruct);
}

/**********************************************************
 * @brief Khởi tạo phần cứng cho SPI2
 **********************************************************/
static inline void Spi_Hw_Init_SPI2(void) {
    // Bật xung nhịp và cấu hình GPIO cho SPI2
    SETUP_SPI_GPIO(SPI2, SPI2_CLOCK_RCC, SPI2_GPIO_RCC, SPI2_GPIO_PORT, 
                   SPI2_SCK_PIN | SPI2_MISO_PIN | SPI2_MOSI_PIN);
    // Cấu hình chân NSS nếu cần
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = SPI2_NSS_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(SPI2_GPIO_PORT, &GPIO_InitStruct);
}

/**********************************************************
 * @brief Bật SPI1
 **********************************************************/
static inline void Spi_Hw_Enable_SPI1(void) {
    SPI_Cmd(SPI1, ENABLE);  // Kích hoạt SPI1
}

/**********************************************************
 * @brief Bật SPI2
 **********************************************************/
static inline void Spi_Hw_Enable_SPI2(void) {
    SPI_Cmd(SPI2, ENABLE);  // Kích hoạt SPI2
}

/**********************************************************
 * @brief Tắt SPI1
 **********************************************************/
static inline void Spi_Hw_Disable_SPI1(void) {
    SPI_Cmd(SPI1, DISABLE);  // Tắt SPI1
}

/**********************************************************
 * @brief Tắt SPI2
 **********************************************************/
static inline void Spi_Hw_Disable_SPI2(void) {
    SPI_Cmd(SPI2, DISABLE);  // Tắt SPI2
}

/**********************************************************
 * @brief Gửi 1 byte dữ liệu qua SPI1
 * @param[in] data Dữ liệu cần gửi
 * @return Giá trị dữ liệu nhận được từ SPI
 **********************************************************/
static inline uint8_t Spi_Hw_TransmitReceive_SPI1(uint8_t data) {
    // Đợi cho đến khi SPI1 sẵn sàng để truyền
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    // Gửi dữ liệu qua SPI1
    SPI_I2S_SendData(SPI1, data);
    // Đợi dữ liệu được nhận
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    // Trả về dữ liệu nhận được
    return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}

/**********************************************************
 * @brief Gửi 1 byte dữ liệu qua SPI2
 * @param[in] data Dữ liệu cần gửi
 * @return Giá trị dữ liệu nhận được từ SPI
 **********************************************************/
static inline uint8_t Spi_Hw_TransmitReceive_SPI2(uint8_t data) {
    // Đợi cho đến khi SPI2 sẵn sàng để truyền
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    // Gửi dữ liệu qua SPI2
    SPI_I2S_SendData(SPI2, data);
    // Đợi dữ liệu được nhận
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    // Trả về dữ liệu nhận được
    return (uint8_t)SPI_I2S_ReceiveData(SPI2);
}

/**********************************************************
 * @brief Cài đặt chân NSS cho SPI1 (bật hoặc tắt)
 * @param[in] state Trạng thái của NSS (SPI_NSS_LOW: kích hoạt, SPI_NSS_HIGH: giải phóng)
 **********************************************************/
static inline void Spi_Hw_SetNSS_SPI1(Spi_NssStateType state) {
    if (state == SPI_NSS_LOW) {
        GPIO_ResetBits(SPI1_GPIO_PORT, SPI1_NSS_PIN);  // Kéo chân NSS xuống thấp
    } else {
        GPIO_SetBits(SPI1_GPIO_PORT, SPI1_NSS_PIN);    // Kéo chân NSS lên cao
    }
}

/**********************************************************
 * @brief Cài đặt chân NSS cho SPI2 (bật hoặc tắt)
 * @param[in] state Trạng thái của NSS (SPI_NSS_LOW: kích hoạt, SPI_NSS_HIGH: giải phóng)
 **********************************************************/
static inline void Spi_Hw_SetNSS_SPI2(Spi_NssStateType state) {
    if (state == SPI_NSS_LOW) {
        GPIO_ResetBits(SPI2_GPIO_PORT, SPI2_NSS_PIN);  // Kéo chân NSS xuống thấp
    } else {
        GPIO_SetBits(SPI2_GPIO_PORT, SPI2_NSS_PIN);    // Kéo chân NSS lên cao
    }
}

/**********************************************************
 * @brief Khởi tạo phần cứng cho SPI1 dựa trên cấu hình từ ConfigPtr
 * @param[in] ConfigPtr Cấu trúc chứa các thông số khởi tạo SPI1
 **********************************************************/
static inline void Spi_Hw_Init_SPI1(const Spi_ConfigType* ConfigPtr) {
    SPI_InitTypeDef SPI_InitStruct;

    // Kiểm tra nếu cấu hình không hợp lệ
    if (ConfigPtr == NULL) {
        return;  // Không thực hiện nếu tham số không hợp lệ
    }

    // Thiết lập cấu hình cho SPI1
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  // Full-duplex
    SPI_InitStruct.SPI_Mode = (ConfigPtr->Mode == SPI_MODE_MASTER) ? SPI_Mode_Master : SPI_Mode_Slave;
    SPI_InitStruct.SPI_DataSize = (ConfigPtr->DataSize == SPI_DATASIZE_8BIT) ? SPI_DataSize_8b : SPI_DataSize_16b;
    SPI_InitStruct.SPI_CPOL = (ConfigPtr->CPOL == SPI_CPOL_Low) ? SPI_CPOL_Low : SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA = (ConfigPtr->CPHA == SPI_CPHA_1Edge) ? SPI_CPHA_1Edge : SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_NSS = (ConfigPtr->NSS == SPI_NSS_Soft) ? SPI_NSS_Soft : SPI_NSS_Hard;
    SPI_InitStruct.SPI_BaudRatePrescaler = ConfigPtr->BaudRate;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;  // Truyền MSB trước
    SPI_InitStruct.SPI_CRCPolynomial = 7;  // Đa thức CRC mặc định

    // Khởi tạo SPI1
    SPI_Init(SPI1, &SPI_InitStruct);

    // Nếu quản lý NSS bằng phần mềm, thiết lập chân NSS
    if (ConfigPtr->NSS == SPI_NSS_Soft) {
        GPIO_ResetBits(SPI1_GPIO_PORT, SPI1_NSS_PIN);  // Kéo chân NSS xuống thấp
    }

    // Bật SPI1
    SPI_Cmd(SPI1, ENABLE);
}

/**********************************************************
 * @brief Khởi tạo phần cứng cho SPI2 dựa trên cấu hình từ ConfigPtr
 * @param[in] ConfigPtr Cấu trúc chứa các thông số khởi tạo SPI2
 **********************************************************/
static inline void Spi_Hw_Init_SPI2(const Spi_ConfigType* ConfigPtr) {
    SPI_InitTypeDef SPI_InitStruct;

    // Kiểm tra nếu cấu hình không hợp lệ
    if (ConfigPtr == NULL) {
        return;  // Không thực hiện nếu tham số không hợp lệ
    }

    // Thiết lập cấu hình cho SPI2
    SPI_InitStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  // Full-duplex
    SPI_InitStruct.SPI_Mode = (ConfigPtr->Mode == SPI_MODE_MASTER) ? SPI_Mode_Master : SPI_Mode_Slave;
    SPI_InitStruct.SPI_DataSize = (ConfigPtr->DataSize == SPI_DATASIZE_8BIT) ? SPI_DataSize_8b : SPI_DataSize_16b;
    SPI_InitStruct.SPI_CPOL = (ConfigPtr->CPOL == SPI_CPOL_Low) ? SPI_CPOL_Low : SPI_CPOL_High;
    SPI_InitStruct.SPI_CPHA = (ConfigPtr->CPHA == SPI_CPHA_1Edge) ? SPI_CPHA_1Edge : SPI_CPHA_2Edge;
    SPI_InitStruct.SPI_NSS = (ConfigPtr->NSS == SPI_NSS_Soft) ? SPI_NSS_Soft : SPI_NSS_Hard;
    SPI_InitStruct.SPI_BaudRatePrescaler = ConfigPtr->BaudRate;
    SPI_InitStruct.SPI_FirstBit = SPI_FirstBit_MSB;  // Truyền MSB trước
    SPI_InitStruct.SPI_CRCPolynomial = 7;  // Đa thức CRC mặc định

    // Khởi tạo SPI2
    SPI_Init(SPI2, &SPI_InitStruct);

    // Nếu quản lý NSS bằng phần mềm, thiết lập chân NSS
    if (ConfigPtr->NSS == SPI_NSS_Soft) {
        GPIO_ResetBits(SPI2_GPIO_PORT, SPI2_NSS_PIN);  // Kéo chân NSS xuống thấp
    }

    // Bật SPI2
    SPI_Cmd(SPI2, ENABLE);
}

/**********************************************************
 * @brief Bật xung nhịp cho SPI1
 **********************************************************/
static inline void Spi_Hw_EnableClock_SPI1(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);  // Bật xung nhịp SPI1
}

/**********************************************************
 * @brief Tắt xung nhịp cho SPI1
 **********************************************************/
static inline void Spi_Hw_DisableClock_SPI1(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);  // Tắt xung nhịp SPI1
}

/**********************************************************
 * @brief Bật xung nhịp cho SPI2
 **********************************************************/
static inline void Spi_Hw_EnableClock_SPI2(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);  // Bật xung nhịp SPI2
}

/**********************************************************
 * @brief Tắt xung nhịp cho SPI2
 **********************************************************/
static inline void Spi_Hw_DisableClock_SPI2(void) {
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, DISABLE);  // Tắt xung nhịp SPI2
}

/**********************************************************
 * @brief Hủy khởi tạo phần cứng SPI1
 **********************************************************/
static inline void Spi_Hw_DeInit_SPI1(void) {
    // Tắt SPI1
    SPI_Cmd(SPI1, DISABLE);  // Tắt SPI1
    Spi_Hw_DisableClock_SPI1();  // Tắt xung nhịp cho SPI1
}

/**********************************************************
 * @brief Hủy khởi tạo phần cứng SPI2
 **********************************************************/
static inline void Spi_Hw_DeInit_SPI2(void) {
    // Tắt SPI2
    SPI_Cmd(SPI2, DISABLE);  // Tắt SPI2
    Spi_Hw_DisableClock_SPI2();  // Tắt xung nhịp cho SPI2
}

/**********************************************************
 * @brief Đợi cho đến khi bộ đệm truyền của SPI1 trống
 **********************************************************/
static inline void Spi_Hw_WaitTransmitBufferEmpty_SPI1(void) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);  // Chờ bộ đệm trống
}

/**********************************************************
 * @brief Gửi dữ liệu qua SPI1
 * @param[in] DataBufferPtr Con trỏ tới dữ liệu cần gửi
 **********************************************************/
static inline void Spi_I2S_SendData_SPI1(const Spi_DataBufferType* DataBufferPtr) {
    SPI_I2S_SendData(SPI1, *DataBufferPtr);  // Gửi dữ liệu qua SPI1
}

/**********************************************************
 * @brief Đợi cho đến khi bộ đệm truyền của SPI2 trống
 **********************************************************/
static inline void Spi_Hw_WaitTransmitBufferEmpty_SPI2(void) {
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);  // Chờ bộ đệm trống
}

/**********************************************************
 * @brief Gửi dữ liệu qua SPI2
 * @param[in] DataBufferPtr Con trỏ tới dữ liệu cần gửi
 **********************************************************/
static inline void Spi_I2S_SendData_SPI2(const Spi_DataBufferType* DataBufferPtr) {
    SPI_I2S_SendData(SPI2, *DataBufferPtr);  // Gửi dữ liệu qua SPI2
}

/**********************************************************
 * @brief Đợi cho đến khi nhận được dữ liệu từ SPI1
 **********************************************************/
static inline void Spi_Hw_WaitReceiveBufferFull_SPI1(void) {
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);  // Đợi dữ liệu từ SPI1
}

/**********************************************************
 * @brief Đọc dữ liệu từ SPI1
 * @return Dữ liệu nhận được từ SPI1
 **********************************************************/
static inline uint16_t Spi_I2S_ReceiveData_SPI1(void) {
    return SPI_I2S_ReceiveData(SPI1);  // Đọc dữ liệu từ SPI1
}

/**********************************************************
 * @brief Đợi cho đến khi nhận được dữ liệu từ SPI2
 **********************************************************/
static inline void Spi_Hw_WaitReceiveBufferFull_SPI2(void) {
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);  // Đợi dữ liệu từ SPI2
}

/**********************************************************
 * @brief Đọc dữ liệu từ SPI2
 * @return Dữ liệu nhận được từ SPI2
 **********************************************************/
static inline uint16_t Spi_I2S_ReceiveData_SPI2(void) {
    return SPI_I2S_ReceiveData(SPI2);  // Đọc dữ liệu từ SPI2
}

/**********************************************************
 * @brief Kiểm tra trạng thái bộ đệm truyền và nhận của SPI1
 * @return SPI_BUSY nếu SPI1 đang bận, SPI_IDLE nếu SPI1 không bận
 **********************************************************/
static inline Spi_StatusType Spi_Hw_CheckStatus_SPI1(void) {
    // Kiểm tra trạng thái của bộ đệm truyền và nhận của SPI1
    if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET || SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) {
        return SPI_BUSY;  // SPI1 đang bận truyền hoặc nhận dữ liệu
    }
    return SPI_IDLE;  // SPI1 không bận
}

/**********************************************************
 * @brief Kiểm tra trạng thái bộ đệm truyền và nhận của SPI2
 * @return SPI_BUSY nếu SPI2 đang bận, SPI_IDLE nếu SPI2 không bận
 **********************************************************/
static inline Spi_StatusType Spi_Hw_CheckStatus_SPI2(void) {
    // Kiểm tra trạng thái của bộ đệm truyền và nhận của SPI2
    if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET || SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) {
        return SPI_BUSY;  // SPI2 đang bận truyền hoặc nhận dữ liệu
    }
    return SPI_IDLE;  // SPI2 không bận
}

/**********************************************************
 * @brief Kiểm tra trạng thái hiện tại của Job trên SPI1
 * @return SPI_JOB_PENDING nếu SPI1 đang thực hiện Job,
 *         SPI_JOB_OK nếu Job đã hoàn thành.
 **********************************************************/
static inline Spi_JobResultType Spi_Hw_CheckJobStatus_SPI1(void) {
    // Kiểm tra nếu SPI1 đang truyền dữ liệu
    if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) {
        return SPI_JOB_PENDING;  // SPI1 vẫn đang thực hiện Job
    }

    // Nếu SPI1 không bận, trả về SPI_JOB_OK
    return SPI_JOB_OK;
}

/**********************************************************
 * @brief Kiểm tra trạng thái hiện tại của Job trên SPI2
 * @return SPI_JOB_PENDING nếu SPI2 đang thực hiện Job,
 *         SPI_JOB_OK nếu Job đã hoàn thành.
 **********************************************************/
static inline Spi_JobResultType Spi_Hw_CheckJobStatus_SPI2(void) {
    // Kiểm tra nếu SPI2 đang truyền dữ liệu
    if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {
        return SPI_JOB_PENDING;  // SPI2 vẫn đang thực hiện Job
    }

    // Nếu SPI2 không bận, trả về SPI_JOB_OK
    return SPI_JOB_OK;
}

**********************************************************
 * @brief Kiểm tra trạng thái hiện tại của một Sequence trên SPI1
 * @return SPI_SEQ_PENDING nếu có Job trong Sequence đang thực hiện,
 *         SPI_SEQ_OK nếu tất cả Job trong Sequence đã hoàn thành.
 **********************************************************/
static inline Spi_SeqResultType Spi_Hw_CheckSequenceStatus_SPI1(void) {
    // Kiểm tra nếu SPI1 đang bận thực hiện một Job trong Sequence
    if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) {
        return SPI_SEQ_PENDING;  // Một Job của Sequence vẫn đang thực hiện
    }

    // Nếu tất cả Job trong Sequence đã hoàn thành, trả về SPI_SEQ_OK
    return SPI_SEQ_OK;
}

/**********************************************************
 * @brief Kiểm tra trạng thái hiện tại của một Sequence trên SPI2
 * @return SPI_SEQ_PENDING nếu có Job trong Sequence đang thực hiện,
 *         SPI_SEQ_OK nếu tất cả Job trong Sequence đã hoàn thành.
 **********************************************************/
static inline Spi_SeqResultType Spi_Hw_CheckSequenceStatus_SPI2(void) {
    // Kiểm tra nếu SPI2 đang bận thực hiện một Job trong Sequence
    if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {
        return SPI_SEQ_PENDING;  // Một Job của Sequence vẫn đang thực hiện
    }

    // Nếu tất cả Job trong Sequence đã hoàn thành, trả về SPI_SEQ_OK
    return SPI_SEQ_OK;
}

/**********************************************************
 * @brief Kiểm tra trạng thái phần cứng của SPI1
 * @return SPI_BUSY nếu SPI1 đang bận, SPI_IDLE nếu SPI1 không bận
 **********************************************************/
static inline Spi_StatusType Spi_Hw_CheckHWStatus_SPI1(void) {
    // Kiểm tra nếu SPI1 đang bận (BSY)
    if (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY) == SET) {
        return SPI_BUSY;  // SPI1 đang bận
    }
    return SPI_IDLE;  // SPI1 không bận
}

/**********************************************************
 * @brief Kiểm tra trạng thái phần cứng của SPI2
 * @return SPI_BUSY nếu SPI2 đang bận, SPI_IDLE nếu SPI2 không bận
 **********************************************************/
static inline Spi_StatusType Spi_Hw_CheckHWStatus_SPI2(void) {
    // Kiểm tra nếu SPI2 đang bận (BSY)
    if (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) == SET) {
        return SPI_BUSY;  // SPI2 đang bận
    }
    return SPI_IDLE;  // SPI2 không bận
}

/**********************************************************
 * @brief Hủy quá trình truyền của SPI1
 * @details Dừng quá trình truyền dữ liệu trên SPI1
 **********************************************************/
static inline void Spi_Hw_Cancel_SPI1(void) {
    // Tắt SPI1 để dừng quá trình truyền
    SPI_Cmd(SPI1, DISABLE);
    // Bật lại SPI1 để sẵn sàng cho các tác vụ khác
    SPI_Cmd(SPI1, ENABLE);
}

/**********************************************************
 * @brief Hủy quá trình truyền của SPI2
 * @details Dừng quá trình truyền dữ liệu trên SPI2
 **********************************************************/
static inline void Spi_Hw_Cancel_SPI2(void) {
    // Tắt SPI2 để dừng quá trình truyền
    SPI_Cmd(SPI2, DISABLE);
    // Bật lại SPI2 để sẵn sàng cho các tác vụ khác
    SPI_Cmd(SPI2, ENABLE);
}

/**********************************************************
 * @brief Chuyển SPI sang chế độ Polling (thăm dò)
 * @details Tắt tất cả các ngắt liên quan đến SPI, bao gồm SPI1 và SPI2.
 *          Chuyển SPI sang chế độ Polling, tức là quá trình truyền và nhận
 *          dữ liệu sẽ không sử dụng ngắt mà sử dụng phương pháp thăm dò trạng thái.
 **********************************************************/
static inline void Spi_Hw_SetPollingMode(void) {
    // Tắt ngắt của SPI1 (truyền và nhận)
    NVIC_DisableIRQ(SPI1_IRQn);  // Tắt ngắt chính của SPI1

    // Nếu có các ngắt phụ liên quan đến truyền hoặc nhận dữ liệu trên SPI1, chúng ta cũng tắt
    NVIC_DisableIRQ(SPI1_RX_IRQn);  // Tắt ngắt nhận dữ liệu của SPI1 (nếu có)
    NVIC_DisableIRQ(SPI1_TX_IRQn);  // Tắt ngắt truyền dữ liệu của SPI1 (nếu có)

    // Tắt ngắt của SPI2 (truyền và nhận)
    NVIC_DisableIRQ(SPI2_IRQn);  // Tắt ngắt chính của SPI2

    // Nếu có các ngắt phụ liên quan đến truyền hoặc nhận dữ liệu trên SPI2, chúng ta cũng tắt
    NVIC_DisableIRQ(SPI2_RX_IRQn);  // Tắt ngắt nhận dữ liệu của SPI2 (nếu có)
    NVIC_DisableIRQ(SPI2_TX_IRQn);  // Tắt ngắt truyền dữ liệu của SPI2 (nếu có)

    // Xử lý bổ sung cho chế độ Polling nếu cần:
    // Ví dụ: có thể cài đặt lại trạng thái bộ đệm SPI, chuẩn bị các tham số khác để sử dụng Polling

    // Đảm bảo rằng SPI đang ở trạng thái IDLE và sẵn sàng cho Polling
    SpiStatus = SPI_IDLE;
}

/**********************************************************
 * @brief Chuyển SPI sang chế độ Interrupt (ngắt)
 * @details Bật tất cả các ngắt liên quan đến SPI, bao gồm SPI1 và SPI2.
 *          Chuyển SPI sang chế độ Interrupt, quá trình truyền và nhận
 *          dữ liệu sẽ sử dụng ngắt để phản hồi nhanh khi có sự kiện.
 **********************************************************/
static inline void Spi_Hw_SetInterruptMode(void) {
    // Bật ngắt của SPI1 (truyền và nhận)
    NVIC_EnableIRQ(SPI1_IRQn);  // Bật ngắt chính của SPI1

    // Nếu có các ngắt phụ liên quan đến truyền hoặc nhận dữ liệu trên SPI1, chúng ta cũng bật
    NVIC_EnableIRQ(SPI1_RX_IRQn);  // Bật ngắt nhận dữ liệu của SPI1 (nếu có)
    NVIC_EnableIRQ(SPI1_TX_IRQn);  // Bật ngắt truyền dữ liệu của SPI1 (nếu có)

    // Bật ngắt của SPI2 (truyền và nhận)
    NVIC_EnableIRQ(SPI2_IRQn);  // Bật ngắt chính của SPI2

    // Nếu có các ngắt phụ liên quan đến truyền hoặc nhận dữ liệu trên SPI2, chúng ta cũng bật
    NVIC_EnableIRQ(SPI2_RX_IRQn);  // Bật ngắt nhận dữ liệu của SPI2 (nếu có)
    NVIC_EnableIRQ(SPI2_TX_IRQn);  // Bật ngắt truyền dữ liệu của SPI2 (nếu có)

    // Xử lý bổ sung cho chế độ Interrupt nếu cần:
    // Ví dụ: cấu hình lại các thanh ghi SPI nếu cần sử dụng ngắt để truyền nhận dữ liệu

    // Đảm bảo rằng SPI đang ở trạng thái IDLE và sẵn sàng cho chế độ Interrupt
    SpiStatus = SPI_IDLE;
}
#endif /* SPI_HW_H */
