/**********************************************************
 * @file Lin.h
 * @brief AUTOSAR LIN Driver Header File
 * @details File này chứa các định nghĩa cần thiết cho LIN driver
 *          theo tiêu chuẩn AUTOSAR.
 * @version 1.0
 * @date 2024-11-01
 * @author HALA Academy
 **********************************************************/

#ifndef LIN_H
#define LIN_H
#include "stm32f10x.h"             /**< @brief Thư viện phần cứng STM32F103 */
#include "Std_Types.h"             /**< @brief Các kiểu dữ liệu chuẩn AUTOSAR */
#include "Lin_GeneralTypes.h"      /**< @brief Các định nghĩa và kiểu dữ liệu chung cho LIN */
#include "Lin_Types.h"


/**********************************************************
 * @typedef Lin_ConfigType
 * @brief Cấu trúc cấu hình cho LIN driver.
 * @details Cấu trúc này chứa các thông tin cần thiết để cấu hình
 *          LIN driver và các thiết lập SFR ảnh hưởng đến các kênh LIN.
 **********************************************************/
typedef struct {
    uint32_t Lin_BaudRate;               /**< @brief Tốc độ truyền của kênh LIN (baud rate). */
    GPIO_TypeDef* Lin_Port;              /**< @brief Port GPIO cho kênh LIN, ví dụ: GPIOA. */
    uint16_t Lin_TxPin;                  /**< @brief Chân Tx cho kênh LIN, ví dụ: GPIO_PIN_9. */
    uint16_t Lin_RxPin;                  /**< @brief Chân Rx cho kênh LIN, ví dụ: GPIO_PIN_10. */
    uint8_t Lin_Channel;                 /**< @brief Số hiệu kênh LIN. */
    FunctionalState Lin_WakeupSupport;   /**< @brief Hỗ trợ chế độ wake-up (TRUE/FALSE). */
    IRQn_Type Lin_IRQn;                  /**< @brief Số hiệu ngắt cho kênh LIN. */
    uint32_t Lin_Prescaler;              /**< @brief Giá trị chia tần số để điều chỉnh tốc độ truyền. */
    uint32_t Lin_Mode;                   /**< @brief Chế độ hoạt động của LIN (0: master, 1: slave). */
    uint8_t Lin_TimeoutDuration;         /**< @brief Thời gian timeout để phát hiện lỗi. */
} Lin_ConfigType;


/**********************************************************
 * @typedef Lin_FramePidType
 * @brief Kiểu đại diện cho ID bảo vệ hợp lệ.
 * @details Đại diện cho các ID bảo vệ hợp lệ được sử dụng bởi Lin_SendFrame().
 **********************************************************/
typedef uint8 Lin_FramePidType;  /**< @brief Phạm vi: 0...0xFE */

/**********************************************************
 * @typedef Lin_FrameCsModelType
 * @brief Enum chỉ định mô hình checksum.
 * @details Chỉ định mô hình checksum được sử dụng cho frame LIN.
 **********************************************************/
typedef enum {
    LIN_ENHANCED_CS,  /**< @brief Mô hình checksum nâng cao */
    LIN_CLASSIC_CS    /**< @brief Mô hình checksum cổ điển */
} Lin_FrameCsModelType;

/**********************************************************
 * @typedef Lin_FrameResponseType
 * @brief Enum chỉ định loại phản hồi của frame LIN.
 * @details Chỉ định liệu bộ xử lý frame cần truyền phần phản hồi hay không.
 **********************************************************/
typedef enum {
    LIN_FRAMERESPONSE_TX,      /**< @brief Phản hồi được tạo từ node này */
    LIN_FRAMERESPONSE_RX,      /**< @brief Phản hồi được tạo từ node khác và liên quan đến node này */
    LIN_FRAMERESPONSE_IGNORE   /**< @brief Phản hồi được tạo từ node khác và không liên quan đến node này */
} Lin_FrameResponseType;

/**********************************************************
 * @typedef Lin_FrameDlType
 * @brief Kiểu chỉ định độ dài dữ liệu.
 * @details Chỉ định số byte dữ liệu SDU để sao chép.
 **********************************************************/
typedef uint8 Lin_FrameDlType;  /**< @brief Phạm vi: 1...8 */

/**********************************************************
 * @typedef Lin_PduType
 * @brief Cấu trúc cung cấp thông tin PDU.
 * @details Cung cấp ID, mô hình checksum, độ dài dữ liệu và con trỏ SDU.
 **********************************************************/
typedef struct {
    Lin_FramePidType Pid;               /**< @brief PID của LIN frame */
    Lin_FrameCsModelType Cs;            /**< @brief Mô hình checksum */
    Lin_FrameResponseType Drc;          /**< @brief Loại phản hồi */
    Lin_FrameDlType Dl;                 /**< @brief Độ dài dữ liệu */
    uint8* SduPtr;                      /**< @brief Con trỏ tới dữ liệu SDU */
} Lin_PduType;

/**********************************************************
 * @typedef Lin_StatusType
 * @brief Enum đại diện cho trạng thái của kênh hoặc frame LIN.
 * @details Trạng thái hoạt động được trả về bởi dịch vụ Lin_GetStatus().
 **********************************************************/
typedef enum {
    LIN_NOT_OK,         /**< @brief Lỗi phát sinh trong quá trình hoạt động */
    LIN_TX_OK,          /**< @brief Truyền thành công */
    LIN_RX_OK,          /**< @brief Nhận phản hồi chính xác */
    LIN_TX_BUSY,        /**< @brief Đang truyền */
    LIN_TX_HEADER_ERROR,/**< @brief Lỗi trong quá trình truyền header */
    LIN_TX_ERROR,       /**< @brief Lỗi trong quá trình truyền phản hồi */
    LIN_RX_BUSY,        /**< @brief Đang nhận dữ liệu */
    LIN_RX_ERROR,       /**< @brief Lỗi trong quá trình nhận dữ liệu */
    LIN_RX_NO_RESPONSE, /**< @brief Không nhận được phản hồi */
    LIN_OPERATIONAL,    /**< @brief Trạng thái hoạt động bình thường */
    LIN_CH_SLEEP        /**< @brief Trạng thái sleep của kênh */
} Lin_StatusType;

/**********************************************************
 * @typedef Lin_SlaveErrorType
 * @brief Enum đại diện cho các lỗi phát sinh trong quá trình phản hồi của node LIN.
 * @details Bao gồm các lỗi như header error, checksum error, no response, v.v.
 **********************************************************/
typedef enum {
    LIN_ERR_HEADER,         /**< @brief Lỗi trong header */
    LIN_ERR_RESP_STOPBIT,   /**< @brief Lỗi framing trong phản hồi */
    LIN_ERR_RESP_CHKSUM,    /**< @brief Lỗi checksum */
    LIN_ERR_RESP_DATABIT,   /**< @brief Lỗi giám sát bit dữ liệu trong phản hồi */
    LIN_ERR_NO_RESP,        /**< @brief Không có phản hồi */
    LIN_ERR_INC_RESP        /**< @brief Phản hồi không hoàn chỉnh */
} Lin_SlaveErrorType;

/**********************************************************
 * @brief Khởi tạo mô-đun LIN.
 * @param Config Con trỏ đến cấu trúc cấu hình LIN.
 **********************************************************/
void Lin_Init(const Lin_ConfigType* Config);

/**********************************************************
 * @brief Kiểm tra sự kiện wake-up cho kênh LIN.
 * @param Channel Kênh LIN cần kiểm tra.
 * @return `E_OK` nếu kiểm tra thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Lin_CheckWakeup(uint8 Channel);

/**********************************************************
 * @brief Lấy thông tin phiên bản của LIN driver.
 * @param versioninfo Con trỏ đến cấu trúc để lưu thông tin phiên bản.
 **********************************************************/
void Lin_GetVersionInfo(Std_VersionInfoType* versioninfo);

/**********************************************************
 * @brief Gửi một frame LIN.
 * @param Channel Kênh LIN cần gửi frame.
 * @param PduInfoPtr Con trỏ đến PDU chứa thông tin để gửi.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Lin_SendFrame(uint8 Channel, const Lin_PduType* PduInfoPtr);

/**********************************************************
 * @brief Chuyển kênh LIN vào chế độ sleep.
 * @param Channel Kênh LIN cần chuyển vào chế độ sleep.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Lin_GoToSleep(uint8 Channel);

/**********************************************************
 * @brief Đặt kênh LIN vào trạng thái sleep và kích hoạt phát hiện wake-up.
 * @param Channel Kênh LIN cần xử lý.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Lin_GoToSleepInternal(uint8 Channel);

/**********************************************************
 * @brief Phát xung wake-up và đặt trạng thái kênh thành LIN_CH_OPERATIONAL.
 * @param Channel Kênh LIN cần phát xung wake-up.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Lin_Wakeup(uint8 Channel);

/**********************************************************
 * @brief Lấy trạng thái hiện tại của kênh LIN.
 * @param Channel Kênh LIN cần kiểm tra.
 * @param Lin_SduPtr Con trỏ tới một con trỏ chứa SDU hiện tại.
 * @return Trạng thái hiện tại của kênh LIN.
 **********************************************************/
Lin_StatusType Lin_GetStatus(uint8 Channel, const uint8** Lin_SduPtr);

#endif /* LIN_H */
