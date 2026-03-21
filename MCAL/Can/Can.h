/**********************************************************
 * @file Can.h
 * @brief AUTOSAR CAN Driver Header File
 * @details File này chứa các định nghĩa và khai báo hàm cho CAN driver 
 *          theo tiêu chuẩn AUTOSAR.
 * @version 1.0
 * @date 2024-10-30
 * @author HALA Academy
 **********************************************************/

#ifndef CAN_H
#define CAN_H

#include "Std_Types.h"
#include "Can_Types.h"
#include "Can_GeneralTypes.h"

/**********************************************************
 * @typedef Can_ConfigType
 * @brief Cấu trúc cấu hình cho CAN driver theo chuẩn AUTOSAR.
 * @details Cấu trúc này chứa các thành phần cần thiết để cấu hình
 *          CAN controller, bao gồm các thông số cơ bản và các tính năng 
 *          bổ sung giúp điều chỉnh tốc độ baud, chế độ hoạt động, 
 *          và các thiết lập nâng cao khác của CAN.
 **********************************************************/
typedef struct {
    /**********************************************************
     * @struct BasicConfig
     * @brief Cấu trúc con cho các thiết lập cơ bản của CAN.
     * @details Cấu trúc này chứa các thông số cơ bản để thiết lập CAN
     *          controller, bao gồm tốc độ truyền, chế độ hoạt động và 
     *          các khoảng thời gian đồng bộ hóa. Các thông số này là
     *          cốt lõi để cấu hình và vận hành CAN đúng với yêu cầu hệ thống.
     **********************************************************/
    struct {
        uint16 CAN_Prescaler;               /**< @brief Giá trị chia tần số để điều chỉnh tốc độ truyền.
                                              *   @details Giá trị này sẽ chia tần số của CAN clock để 
                                              *   đạt được tốc độ truyền mong muốn. Giá trị càng nhỏ thì
                                              *   tốc độ truyền càng nhanh. */
        
        uint8 CAN_Mode;                     /**< @brief Chế độ hoạt động của CAN.
                                              *   @details Các chế độ hoạt động có thể bao gồm:
                                              *   - 0: Normal Mode (Chế độ bình thường).
                                              *   - 1: Loopback Mode (Chế độ vòng lặp để kiểm tra).
                                              *   - 2: Silent Mode (Chế độ im lặng, chỉ nhận dữ liệu). */

        uint8 CAN_SJW;                      /**< @brief Synchronization Jump Width (SJW).
                                              *   @details Độ rộng của khoảng nhảy đồng bộ để điều chỉnh
                                              *   sự sai lệch thời gian của tín hiệu. SJW được dùng để 
                                              *   giảm thiểu lỗi đồng bộ hóa giữa các node CAN.
                                              *   Giá trị có thể từ 1 đến 4. */

        uint8 CAN_BS1;                      /**< @brief Time Segment 1 (BS1).
                                              *   @details Độ dài của đoạn bit đầu tiên trong một bit time,
                                              *   xác định thời gian của segment 1, bao gồm cả thời gian
                                              *   truyền và lấy mẫu tín hiệu. */

        uint8 CAN_BS2;                      /**< @brief Time Segment 2 (BS2).
                                              *   @details Độ dài của đoạn bit thứ hai trong một bit time,
                                              *   giúp ổn định tín hiệu sau khi lấy mẫu và làm giảm nhiễu.
                                              *   Độ dài của BS2 ảnh hưởng đến tính ổn định của tín hiệu. */
    } BasicConfig;   /**< @brief Cấu hình cơ bản cho CAN. */

    /**********************************************************
     * @struct Features
     * @brief Cấu trúc con cho các tính năng bổ sung của CAN.
     * @details Các tính năng này giúp CAN hỗ trợ chế độ hoạt động
     *          nâng cao, tự động quản lý lỗi và đảm bảo tính linh
     *          hoạt cho CAN controller trong các tình huống khác nhau.
     **********************************************************/
    struct {
        FunctionalState CAN_TTCM;           /**< @brief Time Triggered Communication Mode (TTCM).
                                              *   @details Kích hoạt chế độ truyền dữ liệu theo thời gian.
                                              *   Khi TTCM được bật, CAN có thể truyền dữ liệu theo chu kỳ
                                              *   thời gian cố định, giúp đồng bộ các node CAN trong hệ thống. */

        FunctionalState CAN_ABOM;           /**< @brief Automatic Bus-Off Management (ABOM).
                                              *   @details Kích hoạt chế độ tự động quản lý bus-off.
                                              *   ABOM cho phép CAN tự động phục hồi và tái khởi động khi
                                              *   phát hiện lỗi trên bus CAN, đảm bảo hệ thống không bị gián đoạn. */

        FunctionalState CAN_AWUM;           /**< @brief Automatic Wake-Up Mode (AWUM).
                                              *   @details Kích hoạt chế độ wake-up tự động.
                                              *   Khi AWUM được bật, CAN controller có thể tự động khởi động
                                              *   lại từ trạng thái sleep khi nhận thấy có hoạt động trên bus. */

        FunctionalState CAN_NART;           /**< @brief Non-Automatic Retransmission (NART).
                                              *   @details Kích hoạt chế độ không truyền lại tự động.
                                              *   Khi bật NART, CAN sẽ không tự động truyền lại dữ liệu
                                              *   nếu phát hiện lỗi, hữu ích trong các ứng dụng yêu cầu 
                                              *   không trùng lặp dữ liệu. */

        FunctionalState CAN_RFLM;           /**< @brief Receive FIFO Locked Mode (RFLM).
                                              *   @details Kích hoạt chế độ khóa FIFO nhận.
                                              *   Khi RFLM được bật, FIFO nhận sẽ bị khóa khi đầy và 
                                              *   không nhận thêm dữ liệu mới, giúp tránh mất dữ liệu. */

        FunctionalState CAN_TXFP;           /**< @brief Transmit FIFO Priority (TXFP).
                                              *   @details Kích hoạt chế độ ưu tiên FIFO truyền.
                                              *   Khi bật TXFP, các tin nhắn trong FIFO truyền sẽ được 
                                              *   xử lý theo thứ tự nhập vào, đảm bảo tính nhất quán trong truyền dữ liệu. */
    } Features;    /**< @brief Cấu hình các tính năng bổ sung cho CAN. */
} Can_ConfigType;



/**********************************************************
 * @brief Khởi tạo mô-đun CAN với cấu hình được cung cấp.
 * @param Config Cấu hình CAN driver.
 **********************************************************/
void Can_Init(const Can_ConfigType* Config);

/**********************************************************
 * @brief Lấy thông tin phiên bản của CAN driver.
 * @param versioninfo Con trỏ đến struct để lưu thông tin phiên bản.
 **********************************************************/
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo);

/**********************************************************
 * @brief Hủy khởi tạo CAN driver.
 **********************************************************/
void Can_DeInit(void);

/**********************************************************
 * @brief Thiết lập tốc độ truyền cho một CAN controller.
 * @param Controller ID của controller.
 * @param BaudRateConfigID ID cấu hình tốc độ truyền.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_SetBaudrate(uint8 Controller, uint16 BaudRateConfigID);

/**********************************************************
 * @brief Chuyển đổi trạng thái của một CAN controller.
 * @param Controller ID của controller.
 * @param Transition Trạng thái chuyển đổi.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_SetControllerMode(uint8 Controller, Can_ControllerStateType Transition);

/**********************************************************
 * @brief Vô hiệu hóa ngắt cho một CAN controller.
 * @param Controller ID của controller.
 **********************************************************/
void Can_DisableControllerInterrupts(uint8 Controller);

/**********************************************************
 * @brief Kích hoạt ngắt cho một CAN controller.
 * @param Controller ID của controller.
 **********************************************************/
void Can_EnableControllerInterrupts(uint8 Controller);

/**********************************************************
 * @brief Kiểm tra sự kiện đánh thức cho một CAN controller.
 * @param Controller ID của controller.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_CheckWakeup(uint8 Controller);

/**********************************************************
 * @brief Lấy trạng thái lỗi của một CAN controller.
 * @param ControllerId ID của controller.
 * @param ErrorStatePtr Con trỏ để lưu trạng thái lỗi.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetControllerErrorState(uint8 ControllerId, Can_ErrorStateType* ErrorStatePtr);

/**********************************************************
 * @brief Lấy trạng thái hiện tại của một CAN controller.
 * @param Controller ID của controller.
 * @param ControllerModePtr Con trỏ để lưu trạng thái của controller.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetControllerMode(uint8 Controller, Can_ControllerStateType* ControllerModePtr);

/**********************************************************
 * @brief Lấy bộ đếm lỗi Rx của một CAN controller.
 * @param ControllerId ID của controller.
 * @param RxErrorCounterPtr Con trỏ để lưu bộ đếm lỗi Rx.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetControllerRxErrorCounter(uint8 ControllerId, uint8* RxErrorCounterPtr);

/**********************************************************
 * @brief Lấy bộ đếm lỗi Tx của một CAN controller.
 * @param ControllerId ID của controller.
 * @param TxErrorCounterPtr Con trỏ để lưu bộ đếm lỗi Tx.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetControllerTxErrorCounter(uint8 ControllerId, uint8* TxErrorCounterPtr);

/**********************************************************
 * @brief Lấy thời gian hiện tại từ thanh ghi phần cứng.
 * @param ControllerId ID của controller.
 * @param timeStampPtr Con trỏ để lưu thời gian hiện tại.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetCurrentTime(uint8 ControllerId, Can_TimeStampType* timeStampPtr);

/**********************************************************
 * @brief Kích hoạt đánh dấu thời gian đầu ra cho một CAN handle.
 * @param Hth CAN handle cho việc truyền dữ liệu.
 **********************************************************/
void Can_EnableEgressTimeStamp(Can_HwHandleType Hth);

/**********************************************************
 * @brief Lấy thời gian đầu ra cho một PDU.
 * @param TxPduId ID của PDU truyền.
 * @param Hth CAN handle cho việc truyền.
 * @param timeStampPtr Con trỏ để lưu thời gian đầu ra.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetEgressTimeStamp(PduIdType TxPduId, Can_HwHandleType Hth, Can_TimeStampType* timeStampPtr);

/**********************************************************
 * @brief Lấy thời gian đầu vào cho một CAN handle.
 * @param Hrh CAN handle cho việc nhận dữ liệu.
 * @param timeStampPtr Con trỏ để lưu thời gian đầu vào.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Can_GetIngressTimeStamp(Can_HwHandleType Hrh, Can_TimeStampType* timeStampPtr);

#endif /* CAN_H_ */
