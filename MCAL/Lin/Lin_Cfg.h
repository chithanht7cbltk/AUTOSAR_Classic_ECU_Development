/**********************************************************
 * @file Lin_Cfg.h
 * @brief Header file duy nhất cho LIN driver.
 * @details File này chứa các khai báo cấu trúc, hằng số, và mảng cần thiết
 *          để cấu hình và quản lý trạng thái của các kênh LIN, cùng với
 *          các thông tin phiên bản và ID nhà cung cấp/module.
 * @version 1.0
 * @date 2024-11-01
 * @author HALA Academy
 **********************************************************/

#ifndef LIN_CFG_H
#define LIN_CFG_H

#include "stm32f1xx.h"  // Thêm thư viện phần cứng nếu sử dụng STM32
#include "Lin_Types.h"  // Thêm các định nghĩa kiểu dữ liệu cần thiết

/**********************************************************
 * @brief Định nghĩa các trạng thái của kênh LIN.
 **********************************************************/
#define LIN_CH_SLEEP         0  /**< @brief Trạng thái ngủ của kênh LIN. */
#define LIN_CH_OPERATIONAL   1  /**< @brief Trạng thái hoạt động của kênh LIN. */
#define LIN_CH_SLEEP_PENDING 2  /**< @brief Trạng thái đang chờ để chuyển vào sleep. */

/**********************************************************
 * @brief Định nghĩa số lượng kênh LIN tối đa.
 * @details Giá trị này cần được cấu hình dựa trên hệ thống thực tế
 *          và khả năng hỗ trợ của vi điều khiển.
 **********************************************************/
#define MAX_LIN_CHANNELS 2  // Giá trị này có thể thay đổi tùy thuộc vào số lượng kênh LIN mà hệ thống hỗ trợ

/**********************************************************
 * @brief Định nghĩa ID nhà cung cấp (Vendor ID).
 **********************************************************/
#define LIN_VENDOR_ID          123  /**< @brief ID nhà cung cấp cho LIN driver. */

/**********************************************************
 * @brief Định nghĩa ID module (Module ID).
 **********************************************************/
#define LIN_MODULE_ID          456  /**< @brief ID module cho LIN driver. */

/**********************************************************
 * @brief Định nghĩa phiên bản phần mềm (Software Version).
 **********************************************************/
#define LIN_SW_MAJOR_VERSION   1    /**< @brief Phiên bản chính của phần mềm. */
#define LIN_SW_MINOR_VERSION   0    /**< @brief Phiên bản phụ của phần mềm. */
#define LIN_SW_PATCH_VERSION   0    /**< @brief Phiên bản sửa lỗi của phần mềm. */

/**********************************************************
 * @enum Lin_StatusType
 * @brief Các trạng thái khác nhau của kênh LIN.
 **********************************************************/
typedef enum {
    LIN_CH_SLEEP,           /**< @brief Trạng thái ngủ của kênh LIN. */
    LIN_CH_OPERATIONAL,     /**< @brief Trạng thái hoạt động của kênh LIN. */
    LIN_CH_SLEEP_PENDING    /**< @brief Trạng thái đang chờ chuyển sang ngủ. */
} Lin_StatusType;

/**********************************************************
 * @struct LinChannelConfigType
 * @brief Cấu trúc chứa thông tin cấu hình cho một kênh LIN.
 **********************************************************/
typedef struct {
    uint32 Lin_BaudRate;               /**< @brief Tốc độ truyền dữ liệu của kênh LIN. */
    FunctionalState LinChannelWakeupSupport; /**< @brief Hỗ trợ phát hiện wake-up (ENABLE/DISABLE). */
    uint8 Lin_ChannelID;               /**< @brief ID của kênh LIN. */
    GPIO_TypeDef* Lin_Port;            /**< @brief Cổng GPIO của kênh LIN. */
    uint16 Lin_TxPin;                  /**< @brief Chân Tx của kênh LIN. */
    uint16 Lin_RxPin;                  /**< @brief Chân Rx của kênh LIN. */
} LinChannelConfigType;

/**********************************************************
 * @brief Mảng lưu trạng thái của các kênh LIN.
 * @details Mỗi phần tử trong mảng đại diện cho trạng thái của một kênh LIN cụ thể.
 **********************************************************/
Lin_StatusType LinChannelState[MAX_LIN_CHANNELS] = {
    LIN_CH_SLEEP,  // Khởi tạo trạng thái ban đầu cho mỗi kênh
    LIN_CH_SLEEP   // Khởi tạo cho kênh thứ hai nếu cần
};

/**********************************************************
 * @brief Mảng lưu trữ cấu hình cho các kênh LIN.
 * @details Mỗi phần tử trong mảng chứa thông tin cấu hình của một kênh LIN,
 *          bao gồm tốc độ baud, trạng thái hỗ trợ wake-up, và các chân GPIO liên quan.
 **********************************************************/
LinChannelConfigType LinChannelConfig[MAX_LIN_CHANNELS] = {
    {
        .Lin_BaudRate = 19200,                        /**< @brief Tốc độ baud cho kênh LIN. */
        .LinChannelWakeupSupport = ENABLE,            /**< @brief Hỗ trợ wake-up. */
        .Lin_ChannelID = 0,                           /**< @brief ID của kênh LIN. */
        .Lin_Port = GPIOA,                            /**< @brief Cổng GPIO sử dụng. */
        .Lin_TxPin = GPIO_Pin_9,                      /**< @brief Chân Tx của kênh. */
        .Lin_RxPin = GPIO_Pin_10                      /**< @brief Chân Rx của kênh. */
    },
    // Thêm cấu hình cho các kênh khác nếu cần
};

#endif /* LIN_CFG_H */
