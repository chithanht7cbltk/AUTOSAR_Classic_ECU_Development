/**********************************************************
 * @file Spi_Cfg.h
 * @brief Cấu hình mô-đun SPI cho hệ thống AUTOSAR
 * @details Tệp này chứa các định nghĩa và cấu hình cho SPI, bao gồm các Job, Channel và Sequence.
 * @version 1.0
 * @date 2024-10-01
 * @author HALA Academy
 **********************************************************/

#ifndef SPI_CFG_H
#define SPI_CFG_H

#include "Std_Types.h"
#include "Spi_Types.h"
#include "Spi_Hw.h"

/**********************************************************
 * @section Định nghĩa kênh SPI (SPI Channel)
 * Kênh SPI đại diện cho đường truyền dữ liệu giữa MCU và thiết bị ngoại vi.
 **********************************************************/

// Định nghĩa các kênh SPI trong hệ thống
#define SPI_CHANNEL_1    0  /**< SPI Channel 1 - kết nối với cảm biến nhiệt độ */
#define SPI_CHANNEL_2    1  /**< SPI Channel 2 - kết nối với EEPROM */
#define SPI_CHANNEL_3    2  /**< SPI Channel 3 - có thể mở rộng cho các thiết bị khác */

/**********************************************************
 * @section Định nghĩa Job SPI (SPI Job)
 * Job đại diện cho một nhiệm vụ SPI, có thể bao gồm một hoặc nhiều kênh.
 **********************************************************/

// Định nghĩa các Job trong hệ thống
#define SPI_JOB_READ_TEMP_SENSOR    0  /**< Job để đọc dữ liệu từ cảm biến nhiệt độ qua SPI1 */
#define SPI_JOB_WRITE_EEPROM        1  /**< Job để ghi dữ liệu vào EEPROM qua SPI2 */
#define SPI_JOB_READ_EEPROM         2  /**< Job để đọc dữ liệu từ EEPROM qua SPI2 */

/**********************************************************
 * @section Định nghĩa Sequence SPI (SPI Sequence)
 * Sequence đại diện cho một chuỗi các Job cần thực hiện liên tiếp.
 **********************************************************/

// Định nghĩa các Sequence trong hệ thống
#define SPI_SEQUENCE_0              0  /**< Sequence 0: Đọc dữ liệu từ cảm biến nhiệt độ */
#define SPI_SEQUENCE_1              1  /**< Sequence 1: Ghi dữ liệu vào EEPROM */
#define SPI_SEQUENCE_2              2  /**< Sequence 2: Đọc dữ liệu từ EEPROM */

/**********************************************************
 * @section Cấu hình cho từng Job
 * Cấu hình chi tiết cho từng Job, bao gồm kênh, tốc độ, và các tham số khác.
 **********************************************************/

typedef struct {
    Spi_ChannelType Channel;    /**< Kênh SPI được sử dụng cho Job */
    uint32 BaudRate;            /**< Tốc độ Baud rate cho SPI */
    uint8 CPOL;                 /**< Cực tính clock (0 = thấp, 1 = cao) */
    uint8 CPHA;                 /**< Pha clock (0 = cạnh 1, 1 = cạnh 2) */
    uint8 Mode;                 /**< Chế độ Master hoặc Slave (0 = Slave, 1 = Master) */
} Spi_JobConfigType;

/**********************************************************
 * @section Cấu hình chi tiết cho từng Job trong hệ thống
 * Bao gồm thông tin về kênh, tốc độ Baud rate, CPOL, CPHA và chế độ.
 **********************************************************/

// Cấu hình Job đọc từ cảm biến nhiệt độ qua SPI1
const Spi_JobConfigType SpiJobConfig_ReadTempSensor = {
    .Channel = SPI_CHANNEL_1,        // Sử dụng SPI1
    .BaudRate = 1000000,             // Baud rate = 1 MHz
    .CPOL = 0,                       // CPOL = 0 (cực tính thấp)
    .CPHA = 0,                       // CPHA = 0 (cạnh thứ nhất)
    .Mode = 1                        // Master mode
};

// Cấu hình Job ghi dữ liệu vào EEPROM qua SPI2
const Spi_JobConfigType SpiJobConfig_WriteEEPROM = {
    .Channel = SPI_CHANNEL_2,        // Sử dụng SPI2
    .BaudRate = 500000,              // Baud rate = 500 kHz
    .CPOL = 0,                       // CPOL = 0 (cực tính thấp)
    .CPHA = 0,                       // CPHA = 0 (cạnh thứ nhất)
    .Mode = 1                        // Master mode
};

// Cấu hình Job đọc dữ liệu từ EEPROM qua SPI2
const Spi_JobConfigType SpiJobConfig_ReadEEPROM = {
    .Channel = SPI_CHANNEL_2,        // Sử dụng SPI2
    .BaudRate = 500000,              // Baud rate = 500 kHz
    .CPOL = 0,                       // CPOL = 0 (cực tính thấp)
    .CPHA = 0,                       // CPHA = 0 (cạnh thứ nhất)
    .Mode = 1                        // Master mode
};

/**********************************************************
 * @section Cấu hình Sequence SPI
 * Các Sequence xác định chuỗi các Job sẽ được thực thi theo thứ tự.
 **********************************************************/

typedef struct {
    Spi_JobType Jobs[2];     /**< Danh sách các Job trong Sequence */
    uint8 JobCount;          /**< Số lượng Job trong Sequence */
} Spi_SequenceConfigType;

/**********************************************************
 * @section Cấu hình cho các Sequence
 **********************************************************/

// Cấu hình cho Sequence 0: Đọc dữ liệu từ cảm biến nhiệt độ
const Spi_SequenceConfigType SpiSequenceConfig_0 = {
    .Jobs = {SPI_JOB_READ_TEMP_SENSOR},  // Chỉ có Job đọc cảm biến nhiệt độ
    .JobCount = 1
};

// Cấu hình cho Sequence 1: Ghi dữ liệu vào EEPROM
const Spi_SequenceConfigType SpiSequenceConfig_1 = {
    .Jobs = {SPI_JOB_WRITE_EEPROM},      // Chỉ có Job ghi vào EEPROM
    .JobCount = 1
};

// Cấu hình cho Sequence 2: Đọc dữ liệu từ EEPROM
const Spi_SequenceConfigType SpiSequenceConfig_2 = {
    .Jobs = {SPI_JOB_READ_EEPROM},       // Chỉ có Job đọc từ EEPROM
    .JobCount = 1
};

/**********************************************************
 * @brief Thiết lập các giá trị mặc định cho cấu trúc Spi_ConfigType
 * @param[in,out] config Con trỏ đến cấu trúc cấu hình cần thiết lập giá trị mặc định
 * @details Hàm này kiểm tra các trường trong cấu trúc cấu hình,
 *          nếu trường nào chưa được khởi tạo (bằng 0), hàm sẽ gán
 *          giá trị mặc định cho trường đó.
 **********************************************************/
static inline void Spi_SetupDefaultConfig(Spi_ConfigType* config) {
    // Nếu BaudRate không được điền, mặc định là 1 MHz (SPI_BaudRatePrescaler_16)
    if (config->BaudRate == 0) {
        config->BaudRate = SPI_BaudRatePrescaler_16;  // Tốc độ 1 MHz
    }

    // Nếu CPOL không được điền, mặc định là CPOL thấp (SPI_CPOL_LOW)
    if (config->CPOL == 0) {
        config->CPOL = SPI_CPOL_Low;
    }

    // Nếu CPHA không được điền, mặc định là CPHA cạnh thứ nhất (SPI_CPHA_1EDGE)
    if (config->CPHA == 0) {
        config->CPHA = SPI_CPHA_1Edge;
    }

    // Nếu Mode không được điền, mặc định là Master (SPI_MODE_MASTER)
    if (config->Mode == 0) {
        config->Mode = SPI_Mode_Master;
    }

    // Nếu NSS không được điền, mặc định quản lý NSS bằng phần mềm (SPI_NSS_Soft)
    if (config->NSS == 0) {
        config->NSS = SPI_NSS_Soft;
    }

    // Nếu DataSize không được điền, mặc định là 8-bit (SPI_DATASIZE_8BIT)
    if (config->DataSize == 0) {
        config->DataSize = SPI_DataSize_8b;
    }
}

#endif /* SPI_CFG_H */
