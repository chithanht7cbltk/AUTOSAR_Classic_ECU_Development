/**********************************************************
 * @file Can_GeneralTypes.h
 * @brief CAN General Types Header File
 * @details File này chứa các định nghĩa về kiểu dữ liệu và khai báo 
 *          các cấu trúc cho CAN Driver tuân theo chuẩn AUTOSAR.
 * @version 1.0
 * @date 2024-09-30
 * @author HALA Academy
 **********************************************************/

#ifndef CAN_GENERAL_TYPES_H
#define CAN_GENERAL_TYPES_H

#include "Std_Types.h"
#include "ComStack_Types.h"

/**********************************************************
 * @typedef Can_IdType
 * @brief Kiểu dữ liệu đại diện cho CAN ID
 * @details Sử dụng uint32 để thể hiện cả CAN ID 11-bit và 29-bit
 **********************************************************/
typedef uint32 Can_IdType;

/**********************************************************
 * @typedef Can_HwHandleType
 * @brief Kiểu dữ liệu đại diện cho Hardware Handle của CAN
 * @details Giá trị là uint8 nếu số lượng HW object nhỏ hơn 255, 
 *          và uint16 nếu số lượng HW object lớn hơn.
 **********************************************************/
typedef uint16 Can_HwHandleType;

/**********************************************************
 * @typedef Can_ErrorStateType
 * @brief Trạng thái lỗi của CAN Controller
 **********************************************************/
typedef enum {
    CAN_ERRORSTATE_ACTIVE,   /**< Controller tham gia đầy đủ vào giao tiếp */
    CAN_ERRORSTATE_PASSIVE,  /**< Controller tham gia giao tiếp nhưng không gửi khung lỗi */
    CAN_ERRORSTATE_BUSOFF    /**< Controller không tham gia vào giao tiếp */
} Can_ErrorStateType;

/**********************************************************
 * @typedef Can_ControllerStateType
 * @brief Trạng thái của CAN Controller
 **********************************************************/
typedef enum {
    CAN_CS_UNINIT = 0x00,    /**< CAN Controller chưa khởi tạo */
    CAN_CS_STARTED,          /**< CAN Controller đang hoạt động */
    CAN_CS_STOPPED,          /**< CAN Controller dừng hoạt động */
    CAN_CS_SLEEP             /**< CAN Controller ở trạng thái ngủ */
} Can_ControllerStateType;

/**********************************************************
 * @typedef Can_HwType
 * @brief Cấu trúc phần cứng cho một L-PDU trong CAN
 **********************************************************/
typedef struct {
    Can_IdType CanId;               /**< CAN ID 11-bit hoặc 29-bit */
    Can_HwHandleType Hoh;           /**< ID của phần cứng */
    uint8 ControllerId;             /**< ID của CAN Controller */
} Can_HwType;

/**********************************************************
 * @typedef Can_PduType
 * @brief Định nghĩa L-SDU trong CAN
 **********************************************************/
typedef struct {
    PduIdType swPduHandle;          /**< Pdu ID */
    uint8 length;                   /**< Độ dài của dữ liệu */
    Can_IdType id;                  /**< CAN ID (11-bit hoặc 29-bit) */
    uint8 *sdu;               /**< Con trỏ tới dữ liệu L-SDU */
} Can_PduType;

/**********************************************************
 * @typedef Can_TimeStampType
 * @brief Kiểu dữ liệu để lưu timestamp
 * @details Bao gồm giây và nanosecond
 **********************************************************/
typedef struct {
    uint32 seconds;                 /**< Phần giây của timestamp */
    uint32 nanoseconds;             /**< Phần nanosecond của timestamp */
} Can_TimeStampType;

#endif /* CAN_GENERAL_TYPES_H_ */