/**********************************************************
 * @file Lin_GeneralTypes.h
 * @brief Các định nghĩa chung cho LIN driver theo chuẩn AUTOSAR.
 * @details File này chứa các kiểu dữ liệu và định nghĩa chung được sử dụng
 *          trong LIN driver trên STM32F103.
 * @version 1.0
 * @date 2024-11-01
 * @author HALA Academy
 **********************************************************/

#ifndef LIN_GENERAL_TYPES_H
#define LIN_GENERAL_TYPES_H

#include "Std_Types.h"

/**********************************************************
 * @typedef Lin_FramePidType
 * @brief Kiểu đại diện cho ID bảo vệ hợp lệ của LIN frame.
 **********************************************************/
typedef uint8 Lin_FramePidType;  /**< @brief Phạm vi: 0...0xFE */

/**********************************************************
 * @typedef Lin_FrameCsModelType
 * @brief Enum chỉ định mô hình checksum được sử dụng cho frame LIN.
 **********************************************************/
typedef enum {
    LIN_ENHANCED_CS,  /**< @brief Mô hình checksum nâng cao */
    LIN_CLASSIC_CS    /**< @brief Mô hình checksum cổ điển */
} Lin_FrameCsModelType;

/**********************************************************
 * @typedef Lin_FrameResponseType
 * @brief Enum chỉ định loại phản hồi của frame LIN.
 **********************************************************/
typedef enum {
    LIN_FRAMERESPONSE_TX,      /**< @brief Phản hồi được tạo từ node này */
    LIN_FRAMERESPONSE_RX,      /**< @brief Phản hồi được tạo từ node khác và liên quan đến node này */
    LIN_FRAMERESPONSE_IGNORE   /**< @brief Phản hồi được tạo từ node khác và không liên quan đến node này */
} Lin_FrameResponseType;

/**********************************************************
 * @typedef Lin_FrameDlType
 * @brief Kiểu chỉ định độ dài dữ liệu của frame LIN.
 **********************************************************/
typedef uint8 Lin_FrameDlType;  /**< @brief Phạm vi: 1...8 */

/**********************************************************
 * @typedef Lin_StatusType
 * @brief Enum đại diện cho trạng thái của kênh hoặc frame LIN.
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
 **********************************************************/
typedef enum {
    LIN_ERR_HEADER,         /**< @brief Lỗi trong header */
    LIN_ERR_RESP_STOPBIT,   /**< @brief Lỗi framing trong phản hồi */
    LIN_ERR_RESP_CHKSUM,    /**< @brief Lỗi checksum */
    LIN_ERR_RESP_DATABIT,   /**< @brief Lỗi giám sát bit dữ liệu trong phản hồi */
    LIN_ERR_NO_RESP,        /**< @brief Không có phản hồi */
    LIN_ERR_INC_RESP        /**< @brief Phản hồi không hoàn chỉnh */
} Lin_SlaveErrorType;

#endif /* LIN_GENERAL_TYPES_H */
