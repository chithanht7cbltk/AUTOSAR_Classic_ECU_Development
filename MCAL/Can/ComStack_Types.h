/**********************************************************
 * @file ComStack_Types.h
 * @brief Header file cho các kiểu dữ liệu ngăn xếp giao tiếp AUTOSAR
 * @details File chứa các định nghĩa kiểu dữ liệu cho giao thức CAN, LIN, và FlexRay,
 *          hỗ trợ chuẩn hóa truyền thông trong hệ thống nhúng AUTOSAR.
 * @version 1.0
 * @date 2024-10-30
 * @organization HALA Academy
 **********************************************************/

#ifndef COMSTACK_TYPES_H
#define COMSTACK_TYPES_H

#include "Std_Types.h"

/**********************************************************
 * @typedef PduIdType
 * @brief Kiểu dữ liệu đại diện cho ID của PDU
 * @details Là kiểu uint16, có thể thay đổi thành uint32 nếu yêu cầu.
 **********************************************************/
typedef uint16 PduIdType;

/**********************************************************
 * @struct PduInfoType
 * @brief Cấu trúc chứa dữ liệu và độ dài của PDU.
 **********************************************************/
typedef struct {
    uint8* SduDataPtr;       /**< Con trỏ đến dữ liệu tải của PDU */
    PduLengthType SduLength; /**< Độ dài của dữ liệu tải PDU */
} PduInfoType;

/**********************************************************
 * @enum CanTrcv_TrcvModeType
 * @brief Các chế độ hoạt động của CAN Transceiver.
 **********************************************************/
typedef enum {
    CANTRCV_TRCVMODE_NORMAL,    /**< Chế độ bình thường */
    CANTRCV_TRCVMODE_SLEEP,     /**< Chế độ ngủ */
    CANTRCV_TRCVMODE_STANDBY    /**< Chế độ chờ */
} CanTrcv_TrcvModeType;

/**********************************************************
 * @enum CanTrcv_TrcvWakeupReasonType
 * @brief Các lý do đánh thức của CAN Transceiver.
 **********************************************************/
typedef enum {
    CANTRCV_WU_BY_BUS,        /**< Đánh thức bởi tín hiệu trên bus CAN */
    CANTRCV_WU_BY_PIN,        /**< Đánh thức bởi tín hiệu ngoại vi từ chân PIN */
    CANTRCV_WU_INTERNALLY     /**< Đánh thức bởi sự kiện nội bộ */
} CanTrcv_TrcvWakeupReasonType;

/**********************************************************
 * @enum CanTrcv_TrcvWakeupModeType
 * @brief Các chế độ đánh thức của CAN Transceiver.
 **********************************************************/
typedef enum {
    CANTRCV_WU_ENABLE,      /**< Kích hoạt thông báo đánh thức */
    CANTRCV_WU_DISABLE,     /**< Vô hiệu hóa thông báo đánh thức */
    CANTRCV_WU_CLEAR        /**< Xóa các sự kiện đánh thức đã lưu trữ */
} CanTrcv_TrcvWakeupModeType;

/**********************************************************
 * @typedef EcuM_WakeupSourceType
 * @brief Kiểu dữ liệu đại diện cho các nguồn đánh thức trong hệ thống.
 **********************************************************/
typedef uint32 EcuM_WakeupSourceType;

#endif /* COMSTACK_TYPES_H */
