/**********************************************************
 * @file CanIf_Types.h
 * @brief Header file định nghĩa các kiểu dữ liệu cho CAN Interface
 * @details File này chứa các kiểu dữ liệu cấu hình và trạng thái của CAN Interface, 
 *          bao gồm cấu hình PDU, chế độ hoạt động và trạng thái thông báo theo tiêu chuẩn AUTOSAR.
 * 
 * @version 1.0
 * @date 2024-10-30
 * @organization HALA Academy
 **********************************************************/


#ifndef CANIF_TYPES_H
#define CANIF_TYPES_H

#include "Std_Types.h"  // Để đảm bảo tính tương thích với các kiểu chuẩn của AUTOSAR
#include "Can_GeneralTypes.h"

// Định nghĩa các trạng thái của CAN Interface
#define CANIF_UNINITIALIZED 0
#define CANIF_INITIALIZED 1

// Biến toàn cục lưu trạng thái của CAN Interface
uint8 CanIf_InterfaceStatus = CANIF_UNINITIALIZED;


/**********************************************************
 * @enum CanIf_ControllerModeType
 * @brief Các chế độ hoạt động của CAN Controller trong CanIf.
 **********************************************************/
typedef enum {
    CANIF_CS_UNINIT = 0,    /**< Chế độ chưa khởi tạo */
    CANIF_CS_STARTED,       /**< Chế độ đang hoạt động */
    CANIF_CS_STOPPED,       /**< Chế độ dừng */
    CANIF_CS_SLEEP          /**< Chế độ ngủ */
} CanIf_ControllerModeType;



/**********************************************************
 * @enum CanIf_PduModeType
 * @brief Định nghĩa các chế độ của PDU Channel.
 **********************************************************/
typedef enum {
    CANIF_OFFLINE = 0,           /**< Không có hoạt động truyền/nhận */
    CANIF_TX_OFFLINE,            /**< Chỉ tắt đường truyền, đường nhận vẫn hoạt động */
    CANIF_TX_OFFLINE_ACTIVE,     /**< Chế độ chỉ truyền offline */
    CANIF_ONLINE                 /**< Cả đường truyền và nhận đều hoạt động */
} CanIf_PduModeType;

/**********************************************************
 * @enum CanIf_NotifStatusType
 * @brief Trạng thái thông báo cho các sự kiện truyền/nhận của L-PDU.
 **********************************************************/
typedef enum {
    CANIF_NO_NOTIFICATION = 0,   /**< Không có thông báo sự kiện */
    CANIF_TX_RX_NOTIFICATION     /**< Có thông báo sự kiện truyền/nhận */
} CanIf_NotifStatusType;

/**********************************************************
 * @struct CanIf_PduConfigType
 * @brief Cấu hình cho mỗi PDU trong CAN Interface.
 **********************************************************/
typedef struct {
    PduIdType PduId;                   /**< ID của PDU, xác định duy nhất mỗi PDU */
    uint8 PduLength;                   /**< Độ dài của PDU trong bytes */
    CanIf_PduModeType PduMode;         /**< Chế độ hoạt động của PDU (ONLINE, OFFLINE, etc.) */
    Can_IdType CanId;                  /**< CAN ID của PDU (chuẩn hoặc mở rộng) */
    boolean CanIdType;                 /**< Loại CAN ID: TRUE cho ID mở rộng, FALSE cho ID chuẩn */
} CanIf_PduConfigType;

/**********************************************************
 * @struct CanIf_ControllerConfigType
 * @brief Cấu hình cho mỗi CAN Controller.
 **********************************************************/
typedef struct {
    uint8 ControllerId;                /**< ID của CAN Controller */
    uint16 BaudRateConfigID;           /**< ID cấu hình tốc độ truyền baud */
    CanIf_PduModeType DefaultPduMode;  /**< Chế độ mặc định cho PDU */
    boolean isConfigured;              /**< Trạng thái cấu hình của Controller */
} CanIf_ControllerConfigType;

/**********************************************************
 * @struct CanIf_ConfigType
 * @brief Cấu hình tổng thể của CAN Interface.
 * @details Chứa dữ liệu cấu hình của CAN Interface, bao gồm cấu hình
 *          cho các CAN Controller và PDU.
 **********************************************************/
typedef struct {
    CanIf_ControllerConfigType* ControllerConfig; /**< Mảng cấu hình các CAN Controller */
    uint16 NumControllers;                        /**< Số lượng CAN Controllers được cấu hình */
    CanIf_PduConfigType* PduConfig;               /**< Mảng cấu hình các PDUs */
    uint16 NumPdus;                               /**< Số lượng PDUs được cấu hình */
    CanIf_NotifStatusType* NotifStatus;           /**< Mảng trạng thái thông báo của từng PDU */
} CanIf_ConfigType;

#endif /* CANIF_TYPES_H */
