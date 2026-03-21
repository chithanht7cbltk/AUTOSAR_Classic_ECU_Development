/**********************************************************
 * @file Std_Types.h
 * @brief Định nghĩa các kiểu dữ liệu chuẩn cho AUTOSAR
 * @details File này cung cấp các kiểu dữ liệu nền tảng
 *           độc lập với phần cứng và trình biên dịch cho
 *           các module phần mềm cơ bản trong AUTOSAR.
 * @version 1.0
 * @date 2024-09-30
 * @author HALA Academy
 **********************************************************/

#ifndef STD_TYPES_H
#define STD_TYPES_H

/**********************************************************
 *                     INCLUDE GUARD
 * Ngăn chặn việc include nhiều lần, gây lỗi biên dịch
 **********************************************************/

/* ===========================================
 *  Version Information
 * =========================================== */
/**********************************************************
 * @brief Thông tin phiên bản của file Std_Types.h
 * @details Các định nghĩa về phiên bản giúp dễ dàng kiểm soát
 *          các thay đổi và sự tương thích của phần mềm.
 **********************************************************/
#define STD_TYPES_SW_MAJOR_VERSION (1U) /**< Major version number */
#define STD_TYPES_SW_MINOR_VERSION (0U) /**< Minor version number */
#define STD_TYPES_SW_PATCH_VERSION (0U) /**< Patch version number */

/* ===========================================
 *  Platform Independent Data Types
 * =========================================== */
/**********************************************************
 * @brief Các kiểu dữ liệu độc lập với nền tảng
 * @details Định nghĩa rõ ràng về kích thước và dấu của các kiểu dữ liệu.
 *          NOTE: Moved here to be defined before usage in Std_ReturnType.
 **********************************************************/
typedef unsigned char uint8;       /**< Số nguyên không dấu 8-bit */
typedef signed char sint8;         /**< Số nguyên có dấu 8-bit */
typedef unsigned short uint16;     /**< Số nguyên không dấu 16-bit */
typedef signed short sint16;       /**< Số nguyên có dấu 16-bit */
typedef unsigned long uint32;      /**< Số nguyên không dấu 32-bit */
typedef signed long sint32;        /**< Số nguyên có dấu 32-bit */
typedef unsigned long long uint64; /**< Số nguyên không dấu 64-bit */
typedef signed long long sint64;   /**< Số nguyên có dấu 64-bit */

typedef float float32;  /**< Kiểu số thực 32-bit */
typedef double float64; /**< Kiểu số thực 64-bit */

/* ===========================================
 *  Standard Return Type
 * =========================================== */
/**********************************************************
 * @typedef Std_ReturnType
 * @brief Kiểu trả về tiêu chuẩn
 * @details Được sử dụng cho các hàm API, với các giá trị
 *          mặc định là E_OK và E_NOT_OK.
 **********************************************************/
typedef uint8 Std_ReturnType;

#define E_OK 0x00U     /**< Thao tác thành công */
#define E_NOT_OK 0x01U /**< Thao tác thất bại */

/* ===========================================
 *  Logical State Definitions
 * =========================================== */
/**********************************************************
 * @brief Định nghĩa các trạng thái logic cao và thấp
 * @details Được sử dụng cho các tín hiệu đầu vào/đầu ra.
 **********************************************************/
#define STD_HIGH 0x01U /**< Trạng thái logic cao */
#define STD_LOW 0x00U  /**< Trạng thái logic thấp */

/* ===========================================
 *  Null Pointer Definition
 * =========================================== */
/**********************************************************
 * @brief Định nghĩa con trỏ NULL
 * @details Con trỏ NULL là con trỏ trỏ đến địa chỉ 0.
 **********************************************************/
#ifndef NULL_PTR
#define NULL_PTR ((void *)0) /**< Định nghĩa con trỏ NULL */
#endif

/* ===========================================
 *  Volatile Type Definitions
 * =========================================== */
/**********************************************************
 * @brief Các kiểu dữ liệu volatile
 * @details Được sử dụng cho các thanh ghi hoặc vùng nhớ phần cứng.
 **********************************************************/
typedef volatile uint8 vuint8;   /**< Số nguyên không dấu 8-bit volatile */
typedef volatile sint8 vsint8;   /**< Số nguyên có dấu 8-bit volatile */
typedef volatile uint16 vuint16; /**< Số nguyên không dấu 16-bit volatile */
typedef volatile sint16 vsint16; /**< Số nguyên có dấu 16-bit volatile */
typedef volatile uint32 vuint32; /**< Số nguyên không dấu 32-bit volatile */
typedef volatile sint32 vsint32; /**< Số nguyên có dấu 32-bit volatile */
typedef volatile uint64 vuint64; /**< Số nguyên không dấu 64-bit volatile */
typedef volatile sint64 vsint64; /**< Số nguyên có dấu 64-bit volatile */

/* ===========================================
 *  Boolean Type Definitions
 * =========================================== */
/**********************************************************
 * @typedef boolean
 * @brief Kiểu dữ liệu Boolean
 * @details Được sử dụng để biểu diễn giá trị đúng hoặc sai.
 **********************************************************/
typedef uint8 boolean;

#ifndef TRUE
#define TRUE 1U /**< Giá trị Boolean TRUE */
#endif

#ifndef FALSE
#define FALSE 0U /**< Giá trị Boolean FALSE */
#endif

/* ===========================================
 *  Version Information Structure
 * =========================================== */
/**********************************************************
 * @typedef Std_VersionInfoType
 * @brief Cấu trúc thông tin phiên bản
 * @details Cấu trúc này lưu trữ thông tin về phiên bản phần mềm
 *          của một module.
 **********************************************************/
typedef struct {
  uint16 vendorID;        /**< ID nhà cung cấp */
  uint16 moduleID;        /**< ID module */
  uint8 sw_major_version; /**< Phiên bản chính của phần mềm */
  uint8 sw_minor_version; /**< Phiên bản phụ của phần mềm */
  uint8 sw_patch_version; /**< Phiên bản sửa lỗi của phần mềm */
} Std_VersionInfoType;

/* ===========================================
 *  Development Error Tracer (DET) Report Error Macro
 * =========================================== */
/**********************************************************
 * @brief Macro báo lỗi cho Development Error Tracer (DET)
 * @details Được sử dụng để ghi nhận lỗi phát triển trong quá trình chạy.
 **********************************************************/
#define Det_ReportError(ModuleId, InstanceId, ApiId,                           \
                        ErrorId) /* Báo cáo lỗi cho DET */

/* ===========================================
 *  Active/Idle State Definitions
 * =========================================== */
/**********************************************************
 * @brief Định nghĩa trạng thái hoạt động/nhàn rỗi
 * @details Được sử dụng trong các trạng thái hệ thống.
 **********************************************************/
#define STD_ACTIVE 0x01U /**< Trạng thái active */
#define STD_IDLE 0x00U   /**< Trạng thái idle */

/* ===========================================
 *  On/Off State Definitions
 * =========================================== */
/**********************************************************
 * @brief Định nghĩa trạng thái bật/tắt
 * @details Được sử dụng để điều khiển trạng thái bật/tắt.
 **********************************************************/
#define STD_ON 0x01U  /**< Trạng thái ON */
#define STD_OFF 0x00U /**< Trạng thái OFF */

#endif /* STD_TYPES_H */
