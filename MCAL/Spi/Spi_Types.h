/**********************************************************
 * @file Spi_Types.h
 * @brief Định nghĩa các kiểu dữ liệu cho mô-đun SPI trong hệ thống AUTOSAR
 * @details Tệp này chứa các kiểu dữ liệu quan trọng cần thiết cho việc cấu hình
 *và quản lý mô-đun SPI.
 * @version 1.0
 * @date 2024-10-01
 * @author HALA Academy
 **********************************************************/

#ifndef SPI_TYPES_H
#define SPI_TYPES_H

#include "Std_Types.h" // Bao gồm các kiểu dữ liệu chuẩn

/**********************************************************
 * @brief Định nghĩa kiểu dữ liệu cho tốc độ Baud Rate của SPI
 **********************************************************/
typedef uint16 Spi_BaudRateType; /**< Tốc độ Baud rate được biểu diễn dưới dạng
                                    giá trị 16-bit */

/**********************************************************
 * @brief Định nghĩa kiểu dữ liệu cho cực tính Clock (CPOL)
 **********************************************************/
typedef enum {
  SPI_CPOL_LOW = 0, /**< Cực tính Clock thấp khi nhàn rỗi */
  SPI_CPOL_HIGH = 1 /**< Cực tính Clock cao khi nhàn rỗi */
} Spi_ClockPolarityType;

/**********************************************************
 * @brief Định nghĩa kiểu dữ liệu cho pha Clock (CPHA)
 **********************************************************/
typedef enum {
  SPI_CPHA_1EDGE = 0, /**< Lấy mẫu dữ liệu ở cạnh đầu tiên */
  SPI_CPHA_2EDGE = 1  /**< Lấy mẫu dữ liệu ở cạnh thứ hai */
} Spi_ClockPhaseType;

/**********************************************************
 * @brief Định nghĩa kiểu dữ liệu cho chế độ Master/Slave của SPI
 **********************************************************/
typedef enum {
  SPI_MODE_MASTER = 0, /**< Chế độ Master (thiết bị điều khiển) */
  SPI_MODE_SLAVE = 1   /**< Chế độ Slave (thiết bị bị điều khiển) */
} Spi_ModeType;

/**********************************************************
 * @brief Định nghĩa kiểu dữ liệu cho quản lý NSS (Slave Select)
 **********************************************************/
typedef enum {
  SPI_NSS_SOFT = 0, /**< Quản lý NSS bằng phần mềm */
  SPI_NSS_HARD = 1  /**< Quản lý NSS bằng phần cứng */
} Spi_NSSManagementType;

/**********************************************************
 * @brief Định nghĩa kiểu dữ liệu cho kích thước dữ liệu truyền
 **********************************************************/
typedef enum {
  SPI_DATASIZE_8BIT = 0, /**< Truyền dữ liệu 8-bit */
  SPI_DATASIZE_16BIT = 1 /**< Truyền dữ liệu 16-bit */
} Spi_DataSizeType;

/* ===============================
 *  Định nghĩa các kiểu dữ liệu của SPI Handler/Driver
 * =============================== */

/**********************************************************
 * @typedef Spi_ChannelType
 * @brief Kiểu dữ liệu cho kênh SPI
 * @details Xác định ID cho một kênh SPI, dùng để quản lý truyền và nhận dữ
 *liệu.
 **********************************************************/
typedef uint8 Spi_ChannelType;

/**********************************************************
 * @typedef Spi_JobType
 * @brief Kiểu dữ liệu cho Job SPI
 * @details Xác định ID cho một Job SPI. Một Job có thể bao gồm nhiều kênh và
 *thực hiện truyền/nhận dữ liệu.
 **********************************************************/
typedef uint8 Spi_JobType;

/**********************************************************
 * @typedef Spi_SequenceType
 * @brief Kiểu dữ liệu cho Sequence SPI
 * @details Xác định ID cho một Sequence SPI. Một Sequence có thể chứa nhiều Job
 *để thực hiện một tác vụ lớn hơn.
 **********************************************************/
typedef uint8 Spi_SequenceType;

/**********************************************************
 * @typedef Spi_HWUnitType
 * @brief Kiểu dữ liệu cho đơn vị phần cứng SPI
 * @details Xác định đơn vị phần cứng được sử dụng cho giao tiếp SPI trên vi
 *điều khiển.
 **********************************************************/
typedef uint8 Spi_HWUnitType;

/**********************************************************
 * @typedef Spi_StatusType
 * @brief Trạng thái của SPI Handler/Driver
 * @details Xác định trạng thái hiện tại của SPI Handler/Driver, cho biết SPI
 *đang ở trạng thái nào (chưa khởi tạo, bận, hay nhàn rỗi).
 **********************************************************/
typedef enum {
  SPI_UNINIT = 0x00, /**< SPI chưa được khởi tạo */
  SPI_IDLE = 0x01,   /**< SPI đang nhàn rỗi */
  SPI_BUSY = 0x02    /**< SPI đang bận */
} Spi_StatusType;

/**********************************************************
 * @typedef Spi_JobResultType
 * @brief Kết quả của một Job SPI
 * @details Trạng thái hoàn thành của một Job SPI, giúp xác định Job có thành
 *công hay không.
 **********************************************************/
typedef enum {
  SPI_JOB_OK = 0x00,      /**< Job hoàn thành thành công */
  SPI_JOB_PENDING = 0x01, /**< Job đang chờ xử lý */
  SPI_JOB_FAILED = 0x02,  /**< Job thất bại */
  SPI_JOB_QUEUED = 0x03   /**< Job đang trong hàng đợi */
} Spi_JobResultType;

/**********************************************************
 * @typedef Spi_SeqResultType
 * @brief Kết quả của một Sequence SPI
 * @details Trạng thái hoàn thành của một Sequence SPI, cho biết Sequence đã
 *thực hiện xong hay chưa.
 **********************************************************/
typedef enum {
  SPI_SEQ_OK = 0x00,      /**< Sequence hoàn thành thành công */
  SPI_SEQ_PENDING = 0x01, /**< Sequence đang chờ xử lý */
  SPI_SEQ_FAILED = 0x02,  /**< Sequence thất bại */
  SPI_SEQ_CANCELED = 0x03 /**< Sequence bị hủy */
} Spi_SeqResultType;

/**********************************************************
 * @typedef Spi_DataBufferType
 * @brief Kiểu dữ liệu cho buffer chứa dữ liệu SPI
 * @details Đây là kiểu dữ liệu được sử dụng để lưu trữ dữ liệu SPI
 *          được truyền hoặc nhận qua một kênh SPI.
 **********************************************************/
typedef uint8 Spi_DataBufferType;

/**********************************************************
 * @typedef Spi_NumberOfDataType
 * @brief Kiểu dữ liệu cho số lượng phần tử dữ liệu
 * @details Xác định số lượng dữ liệu cần truyền hoặc nhận trong một kênh SPI.
 **********************************************************/
typedef uint16 Spi_NumberOfDataType;

/**********************************************************
 * @typedef Spi_AsyncModeType
 * @brief Kiểu dữ liệu xác định chế độ truyền không đồng bộ của SPI
 * @details Xác định chế độ hoạt động không đồng bộ (polling hoặc interrupt) cho
 *SPI.
 **********************************************************/
typedef enum {
  SPI_POLLING_MODE = 0x00,  /**< Chế độ polling */
  SPI_INTERRUPT_MODE = 0x01 /**< Chế độ interrupt */
} Spi_AsyncModeType;

/**********************************************************
 * @brief Cấu trúc dữ liệu cho cấu hình SPI
 * @details Ba thông số đầu tiên (Channel, Job, Sequence) là bắt buộc,
 *          các thông số sau sẽ có giá trị mặc định nếu không được cung cấp.
 **********************************************************/
typedef struct {
  Spi_ChannelType Channel;   /**< Kênh SPI (bắt buộc) */
  Spi_JobType Job;           /**< Job SPI (bắt buộc) */
  Spi_SequenceType Sequence; /**< Sequence SPI (bắt buộc) */

  // Các thông số sau là tùy chọn, có thể để giá trị mặc định
  Spi_BaudRateType BaudRate; /**< Tốc độ Baud rate (tùy chọn, mặc định 1 MHz) */
  Spi_ClockPolarityType
      CPOL;                /**< Cực tính Clock (tùy chọn, mặc định CPOL = 0) */
  Spi_ClockPhaseType CPHA; /**< Pha Clock (tùy chọn, mặc định CPHA = 0) */
  Spi_ModeType Mode;       /**< Chế độ SPI (tùy chọn, mặc định Master) */
  Spi_NSSManagementType NSS; /**< Quản lý NSS (tùy chọn, mặc định phần mềm) */
  Spi_DataSizeType
      DataSize; /**< Kích thước dữ liệu (tùy chọn, mặc định 8-bit) */
} Spi_ConfigType;

#endif /* SPI_TYPES_H */
