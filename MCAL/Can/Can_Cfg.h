/**********************************************************
 * @file Can_Cfg.h
 * @brief CAN Configuration Header File
 * @details File này chứa các định nghĩa và cấu hình cho CAN Driver
 *          theo chuẩn AUTOSAR.
 * @version 1.0
 * @date 2024-09-30
 * @author HALA Academy
 **********************************************************/

#ifndef CAN_CFG_H
#define CAN_CFG_H

#include "Std_Types.h"

/**********************************************************
 * @brief Bật chế độ mặt nạ riêng lẻ.
 **********************************************************/
#define CAN_ENABLE_INDIVIDUAL_MASK    STD_OFF

/**********************************************************
 * @brief Đặt số lượng msgboxes được sử dụng.
 **********************************************************/
#define CAN_NUM_MSGBOXES              3

/**********************************************************
 * @brief Bật chế độ phát hiện lỗi phát triển.
 **********************************************************/
#define CAN_DEV_ERROR_DETECT          STD_ON

/**********************************************************
 * @brief Kích hoạt API thông tin phiên bản.
 **********************************************************/
#define CAN_VERSION_INFO_API          STD_ON

/**********************************************************
 * @brief Tắt chế độ truyền ghép kênh.
 **********************************************************/
#define CAN_MULTIPLEXED_TRANSMISSION  STD_OFF

/**********************************************************
 * @brief Bật chức năng hủy truyền tin nhắn với mức độ ưu tiên thấp hơn.
 **********************************************************/
#define CAN_HW_TRANSMIT_CANCELLATION  STD_ON

/**********************************************************
 * @brief Không hủy các tin nhắn có mức độ ưu tiên bằng nhau.
 **********************************************************/
#define CAN_IDENTICAL_ID_CANCELLATION STD_OFF

/**********************************************************
 * @brief Thời gian chờ tối đa để chuyển trạng thái CAN (đơn vị: ticks).
 **********************************************************/
#define CAN_TIMEOUT_DURATION          1000  // Ticks

/**********************************************************
 * @brief Bộ đếm thời gian được sử dụng cho tính toán thời gian chờ.
 **********************************************************/
#define CAN_OS_COUNTER                0

/**********************************************************
 * @brief Tần số tham chiếu đồng hồ CPU cho bộ điều khiển CAN.
 **********************************************************/
#define CAN_CPU_CLOCK_REFERENCE       8000000

/**********************************************************
 * @brief Địa chỉ cơ sở cho module điều khiển CAN.
 **********************************************************/
#define CAN_CONTROLLER_BASE_ADDRESS   ((uint32)0x40006400)


/**********************************************************
 * @typedef Can_Arc_ControllerIdType
 * @brief Enum xác định ID của các bộ điều khiển CAN.
 **********************************************************/
typedef enum {
    CAN_CONTROLLER_1 = 0,  // Chỉ có một bộ điều khiển CAN
    CAN_NUM_CONTROLLERS
} Can_Arc_ControllerIdType;

/**********************************************************
 * @typedef Can_Arc_HTHType
 * @brief Enum xác định các handle phần cứng cho truyền tin (HTH).
 **********************************************************/
typedef enum {
    CAN_HTH_0 = 0,  // Handle truyền tin 0
    CAN_HTH_1,      // Handle truyền tin 1
    CAN_HTH_2,      // Handle truyền tin 2
    NUM_OF_HTHS
} Can_Arc_HTHType;

/**********************************************************
 * @typedef Can_Arc_HRHType
 * @brief Enum xác định các handle phần cứng cho nhận tin (HRH).
 **********************************************************/
typedef enum {
    CAN_HRH_0 = 0,  // Handle nhận tin 0
    CAN_HRH_1,      // Handle nhận tin 1
    CAN_HRH_2,      // Handle nhận tin 2
    NUM_OF_HRHS
} Can_Arc_HRHType;

/**********************************************************
 * @brief Cấu hình CAN.
 **********************************************************/
extern const struct Can_ConfigType CanConfigData;

/**********************************************************
 * @brief Chức năng ghi CAN.
 * @param[in] hth Hardware Transmit Handle
 **********************************************************/
void Can_Arc_Write(Can_HwHandleType hth);

/**********************************************************
 * @brief Chức năng đọc CAN.
 * @param[in] hrh Hardware Receive Handle
 **********************************************************/
void Can_Arc_Read(Can_HwHandleType hrh);

/**********************************************************
 * @brief Xử lý sự kiện BusOff của CAN.
 * @param[in] controller ID của bộ điều khiển CAN
 **********************************************************/
void Can_Arc_BusOff(uint8 controller);

/**********************************************************
 * @brief Hàm xử lý ghi chính của CAN.
 * @param[in] controller ID của bộ điều khiển CAN
 **********************************************************/
void Can_Arc_MainFunction_Write(uint8 controller);

/**********************************************************
 * @brief Hàm xử lý ghi tổng quát cho tất cả các bộ điều khiển không có ISR.
 **********************************************************/
static inline void Can_MainFunction_Write(void) {
    Can_Arc_MainFunction_Write(CAN_CONTROLLER_1);
}

/**********************************************************
 * @brief Hàm xử lý đọc chính của CAN.
 * @param[in] controller ID của bộ điều khiển CAN
 **********************************************************/
void Can_Arc_MainFunction_Read(uint8 controller);

/**********************************************************
 * @brief Hàm xử lý đọc tổng quát cho tất cả các bộ điều khiển không có ISR.
 **********************************************************/
static inline void Can_MainFunction_Read(void) {
    Can_Arc_MainFunction_Read(CAN_CONTROLLER_1);
}

/**********************************************************
 * @brief Hàm xử lý sự kiện BusOff tổng quát cho tất cả các bộ điều khiển không có ISR.
 **********************************************************/
static inline void Can_MainFunction_BusOff(void) {
    Can_Arc_BusOff(CAN_CONTROLLER_1);
}

#endif /* CAN_CFG_H_ */
