/**********************************************************
 * @file Spi.h
 * @brief Khai báo các hàm và kiểu dữ liệu liên quan đến giao tiếp SPI
 * @details File này cung cấp các khai báo và định nghĩa cần thiết cho việc khởi tạo,
 *          truyền dữ liệu và quản lý trạng thái giao tiếp SPI trên vi điều khiển STM32.
 * @version 1.0
 * @date 2024-10-01
 * @author HALA Academy
 **********************************************************/

#ifndef SPI_H
#define SPI_H

#include "Spi_Types.h"
#include "Spi_Hw.h"

/* ===============================
 *  Khai báo các hàm API của SPI Handler/Driver
 * =============================== */

/**********************************************************
 * @brief Khởi tạo SPI Handler/Driver với cấu hình đã cho
 * @param[in] ConfigPtr Con trỏ tới cấu trúc cấu hình Spi_ConfigType
 * @details Hàm này khởi tạo các thành phần của SPI, bao gồm các kênh và phần cứng SPI.
 **********************************************************/
void Spi_Init(const Spi_ConfigType* ConfigPtr);

/**********************************************************
 * @brief Hủy khởi tạo SPI Handler/Driver
 * @return E_OK nếu thành công, E_NOT_OK nếu lỗi
 * @details Hủy khởi tạo và giải phóng tài nguyên liên quan đến SPI.
 **********************************************************/
Std_ReturnType Spi_DeInit(void);

/**********************************************************
 * @brief Ghi dữ liệu vào kênh nội bộ của SPI
 * @param[in] Channel ID của kênh SPI
 * @param[in] DataBufferPtr Con trỏ tới buffer chứa dữ liệu cần ghi
 * @return E_OK nếu ghi thành công, E_NOT_OK nếu có lỗi
 * @details Ghi dữ liệu vào bộ đệm nội bộ của SPI Handler/Driver để truyền dữ liệu ra ngoài.
 **********************************************************/
Std_ReturnType Spi_WriteIB(Spi_ChannelType Channel, const Spi_DataBufferType* DataBufferPtr);

/**********************************************************
 * @brief Truyền dữ liệu không đồng bộ
 * @param[in] Sequence ID của Sequence cần truyền
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Truyền dữ liệu không đồng bộ qua Sequence SPI, không đợi quá trình truyền kết thúc.
 **********************************************************/
Std_ReturnType Spi_AsyncTransmit(Spi_SequenceType Sequence);

/**********************************************************
 * @brief Đọc dữ liệu từ kênh nội bộ của SPI
 * @param[in] Channel ID của kênh SPI
 * @param[out] DataBufferPtr Con trỏ tới buffer để lưu dữ liệu nhận được
 * @return E_OK nếu đọc thành công, E_NOT_OK nếu có lỗi
 * @details Đọc dữ liệu từ buffer nội bộ của kênh SPI.
 **********************************************************/
Std_ReturnType Spi_ReadIB(Spi_ChannelType Channel, Spi_DataBufferType* DataBufferPtr);

/**********************************************************
 * @brief Cấu hình bộ đệm ngoài cho kênh SPI
 * @param[in] Channel ID của kênh SPI
 * @param[in] SrcDataBufferPtr Con trỏ tới buffer nguồn
 * @param[out] DesDataBufferPtr Con trỏ tới buffer đích
 * @param[in] Length Số lượng phần tử dữ liệu cần truyền
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Cấu hình buffer ngoài cho kênh SPI để thực hiện truyền/nhận dữ liệu.
 **********************************************************/
Std_ReturnType Spi_SetupEB(Spi_ChannelType Channel, const Spi_DataBufferType* SrcDataBufferPtr, Spi_DataBufferType* DesDataBufferPtr, Spi_NumberOfDataType Length);

/**********************************************************
 * @brief Lấy trạng thái hiện tại của SPI Handler/Driver
 * @return Trạng thái hiện tại của SPI (SPI_UNINIT, SPI_IDLE, SPI_BUSY)
 * @details Hàm này trả về trạng thái hiện tại của SPI, cho biết SPI đang ở trạng thái nào.
 **********************************************************/
Spi_StatusType Spi_GetStatus(void);

/**********************************************************
 * @brief Lấy kết quả của một Job SPI
 * @param[in] Job ID của Job SPI
 * @return Kết quả của Job (SPI_JOB_OK, SPI_JOB_PENDING, SPI_JOB_FAILED, SPI_JOB_QUEUED)
 * @details Hàm này trả về kết quả của một Job SPI đã được truyền.
 **********************************************************/
Spi_JobResultType Spi_GetJobResult(Spi_JobType Job);

/**********************************************************
 * @brief Lấy kết quả của một Sequence SPI
 * @param[in] Sequence ID của Sequence SPI
 * @return Kết quả của Sequence (SPI_SEQ_OK, SPI_SEQ_PENDING, SPI_SEQ_FAILED, SPI_SEQ_CANCELED)
 * @details Hàm này trả về kết quả của một Sequence SPI đã được truyền.
 **********************************************************/
Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence);

/**********************************************************
 * @brief Lấy thông tin phiên bản của SPI Handler/Driver
 * @param[out] VersionInfo Con trỏ tới cấu trúc Std_VersionInfoType chứa thông tin phiên bản
 * @details Hàm này trả về thông tin phiên bản của SPI Handler/Driver.
 **********************************************************/
void Spi_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**********************************************************
 * @brief Truyền dữ liệu đồng bộ qua SPI
 * @param[in] Sequence ID của Sequence cần truyền
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Truyền dữ liệu đồng bộ qua Sequence SPI và đợi quá trình truyền hoàn thành.
 **********************************************************/
Std_ReturnType Spi_SyncTransmit(Spi_SequenceType Sequence);

/**********************************************************
 * @brief Lấy trạng thái phần cứng của đơn vị SPI
 * @return Trạng thái hiện tại của phần cứng SPI
 * @details Hàm này trả về trạng thái của phần cứng SPI.
 **********************************************************/
Spi_StatusType Spi_GetHWUnitStatus(void);

/**********************************************************
 * @brief Hủy một Sequence đang truyền
 * @param[in] Sequence ID của Sequence cần hủy
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Hủy quá trình truyền của một Sequence hiện tại.
 **********************************************************/
Std_ReturnType Spi_Cancel(Spi_SequenceType Sequence);

/**********************************************************
 * @brief Cài đặt chế độ truyền không đồng bộ cho SPI
 * @param[in] Mode Chế độ truyền không đồng bộ (SPI_POLLING_MODE hoặc SPI_INTERRUPT_MODE)
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Cài đặt chế độ không đồng bộ cho quá trình truyền dữ liệu của SPI.
 **********************************************************/
Std_ReturnType Spi_SetAsyncMode(Spi_AsyncModeType Mode);

/**********************************************************
 * @brief Hàm chính xử lý các tác vụ của SPI
 * @details Hàm này được gọi định kỳ để xử lý các Job và Sequence của SPI.
 **********************************************************/
void Spi_MainFunction_Handling(void);

#endif // SPI_H

