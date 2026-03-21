/**********************************************************
 * @file CanIf.h
 * @brief Header file cho CAN Interface của AUTOSAR
 * @details Định nghĩa các hàm và cấu hình cho CAN Interface, bao gồm
 *          khởi tạo, điều khiển chế độ, và quản lý truyền/nhận dữ liệu
 *          theo tiêu chuẩn AUTOSAR.
 * 
 * @version 1.0
 * @date 2024-10-30
 * @organization HALA Academy
 **********************************************************/


#ifndef CANIF_H
#define CANIF_H

#include "Std_Types.h"
#include "CanIf_Types.h"
#include "Can_GeneralTypes.h"

/**********************************************************
 * @brief Khởi tạo CAN Interface với cấu hình từ CanIf_ConfigType.
 * @param ConfigPtr Con trỏ đến cấu hình CAN Interface.
 **********************************************************/
void CanIf_Init(const CanIf_ConfigType* ConfigPtr);

/**********************************************************
 * @brief Hủy khởi tạo CAN Interface, giải phóng tài nguyên đã sử dụng.
 **********************************************************/
void CanIf_DeInit(void);

/**********************************************************
 * @brief Thay đổi chế độ hoạt động của một CAN Controller.
 * @param ControllerId ID của CAN controller.
 * @param ControllerMode Chế độ yêu cầu của CAN controller.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_SetControllerMode(uint8 ControllerId, Can_ControllerStateType ControllerMode);

/**********************************************************
 * @brief Lấy trạng thái hiện tại của CAN Controller.
 * @param ControllerId ID của CAN controller.
 * @param ControllerModePtr Con trỏ lưu trạng thái hiện tại của CAN controller.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetControllerMode(uint8 ControllerId, Can_ControllerStateType* ControllerModePtr);

/**********************************************************
 * @brief Nhận trạng thái lỗi của CAN Controller.
 * @param ControllerId ID của CAN controller.
 * @param ErrorStatePtr Con trỏ lưu trạng thái lỗi.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetControllerErrorState(uint8 ControllerId, Can_ErrorStateType* ErrorStatePtr);

/**********************************************************
 * @brief Gửi PDU qua CAN Interface.
 * @param TxPduId ID của PDU cần truyền.
 * @param PduInfoPtr Con trỏ đến dữ liệu PDU và metadata.
 * @return `E_OK` nếu truyền thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr);

/**********************************************************
 * @brief Đọc dữ liệu PDU nhận được.
 * @param RxPduId ID của PDU nhận.
 * @param PduInfoPtr Con trỏ đến cấu trúc lưu dữ liệu nhận.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_ReadRxPduData(PduIdType RxPduId, PduInfoType* PduInfoPtr);

/**********************************************************
 * @brief Đọc trạng thái thông báo truyền PDU.
 * @param TxPduId ID của PDU truyền.
 * @return Trạng thái thông báo truyền của PDU (CanIf_NotifStatusType).
 **********************************************************/
CanIf_NotifStatusType CanIf_ReadTxNotifStatus(PduIdType TxPduId);

/**********************************************************
 * @brief Đọc trạng thái thông báo nhận PDU.
 * @param RxPduId ID của PDU nhận.
 * @return Trạng thái thông báo nhận của PDU (CanIf_NotifStatusType).
 **********************************************************/
CanIf_NotifStatusType CanIf_ReadRxNotifStatus(PduIdType RxPduId);

/**********************************************************
 * @brief Thiết lập chế độ hoạt động cho PDU Channel.
 * @param ControllerId ID của CAN controller.
 * @param PduModeRequest Chế độ yêu cầu cho PDU.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_SetPduMode(uint8 ControllerId, CanIf_PduModeType PduModeRequest);

/**********************************************************
 * @brief Lấy chế độ hiện tại của PDU Channel.
 * @param ControllerId ID của CAN controller.
 * @param PduModePtr Con trỏ lưu chế độ PDU hiện tại.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetPduMode(uint8 ControllerId, CanIf_PduModeType* PduModePtr);

/**********************************************************
 * @brief Trả về thông tin phiên bản của mô-đun CAN Interface.
 * @param VersionInfo Con trỏ lưu trữ thông tin phiên bản.
 **********************************************************/
void CanIf_GetVersionInfo(Std_VersionInfoType* VersionInfo);

/**********************************************************
 * @brief Cấu hình lại CAN ID của PDU động.
 * @param CanIfTxSduId ID của PDU truyền động.
 * @param CanId ID chuẩn hoặc mở rộng của CAN.
 **********************************************************/
void CanIf_SetDynamicTxId(PduIdType CanIfTxSduId, Can_IdType CanId);

/**********************************************************
 * @brief Thay đổi chế độ hoạt động của CAN Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @param TransceiverMode Chế độ yêu cầu cho Transceiver.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_SetTrcvMode(uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode);

/**********************************************************
 * @brief Lấy chế độ hiện tại của CAN Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @param TransceiverModePtr Con trỏ lưu trữ chế độ hiện tại của Transceiver.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetTrcvMode(uint8 TransceiverId, CanTrcv_TrcvModeType* TransceiverModePtr);

/**********************************************************
 * @brief Lấy lý do đánh thức của CAN Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @param TrcvWuReasonPtr Con trỏ lưu lý do đánh thức.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetTrcvWakeupReason(uint8 TransceiverId, CanTrcv_TrcvWakeupReasonType* TrcvWuReasonPtr);

/**********************************************************
 * @brief Thiết lập chế độ đánh thức cho CAN Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @param TrcvWakeupMode Chế độ đánh thức yêu cầu.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_SetTrcvWakeupMode(uint8 TransceiverId, CanTrcv_TrcvWakeupModeType TrcvWakeupMode);

/**********************************************************
 * @brief Kiểm tra xem thiết bị CAN Controller hoặc CAN Transceiver đã báo hiệu sự kiện đánh thức chưa.
 * @param WakeupSource Nguồn thiết bị đã kích hoạt sự kiện đánh thức.
 * @return `E_OK` nếu yêu cầu kiểm tra được chấp nhận, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_CheckWakeup(EcuM_WakeupSourceType WakeupSource);

/**********************************************************
 * @brief Xác thực sự kiện đánh thức đã xảy ra trước đó.
 * @param WakeupSource Nguồn thiết bị đã kích hoạt sự kiện đánh thức.
 * @return `E_OK` nếu yêu cầu kiểm tra được chấp nhận, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_CheckValidation(EcuM_WakeupSourceType WakeupSource);

/**********************************************************
 * @brief Báo cáo trạng thái xác nhận của tất cả các PDU truyền từ Controller kể từ lần khởi động gần nhất.
 * @param ControllerId ID của CAN Controller.
 * @return Trạng thái xác nhận kết hợp của tất cả các PDU truyền từ Controller.
 **********************************************************/
CanIf_NotifStatusType CanIf_GetTxConfirmationState(uint8 ControllerId);

/**********************************************************
 * @brief Xóa cờ Wake-up của CAN Transceiver được chỉ định.
 * @param TransceiverId ID của CAN Transceiver.
 * @return `E_OK` nếu yêu cầu được chấp nhận, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_ClearTrcvWufFlag(uint8 TransceiverId);

/**********************************************************
 * @brief Yêu cầu kiểm tra cờ đánh thức của CAN Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @return `E_OK` nếu yêu cầu được chấp nhận, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_CheckTrcvWakeFlag(uint8 TransceiverId);

/**********************************************************
 * @brief Thiết lập cấu hình tốc độ baud của Controller.
 * @param ControllerId ID của CAN Controller.
 * @param BaudRateConfigID ID cấu hình tốc độ baud.
 * @return `E_OK` nếu yêu cầu thiết lập tốc độ baud mới được chấp nhận, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_SetBaudrate(uint8 ControllerId, uint16 BaudRateConfigID);

/**********************************************************
 * @brief Lấy bộ đếm lỗi nhận của Controller.
 * @param ControllerId ID của CAN Controller.
 * @param RxErrorCounterPtr Con trỏ để lưu bộ đếm lỗi nhận.
 * @return `E_OK` nếu có bộ đếm lỗi nhận, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_GetControllerRxErrorCounter(uint8 ControllerId, uint8* RxErrorCounterPtr);

/**********************************************************
 * @brief Lấy bộ đếm lỗi truyền của Controller.
 * @param ControllerId ID của CAN Controller.
 * @param TxErrorCounterPtr Con trỏ để lưu bộ đếm lỗi truyền.
 * @return `E_OK` nếu có bộ đếm lỗi truyền, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_GetControllerTxErrorCounter(uint8 ControllerId, uint8* TxErrorCounterPtr);

/**********************************************************
 * @brief Kích hoạt hoặc vô hiệu hóa tính năng sao chép dữ liệu (mirroring) trên CAN controller được chỉ định.
 * @param ControllerId ID của CAN controller.
 * @param MirroringActive Trạng thái kích hoạt sao chép dữ liệu: TRUE để kích hoạt, FALSE để vô hiệu hóa.
 * @return `E_OK` nếu thao tác thành công, `E_NOT_OK` nếu không thành công.
 **********************************************************/
Std_ReturnType CanIf_EnableBusMirroring(uint8 ControllerId, boolean MirroringActive);

/**********************************************************
 * @brief Lấy dấu thời gian hiện tại từ các thanh ghi phần cứng của CAN Controller.
 * @param Controller ID của CAN controller.
 * @param timeStampPtr Con trỏ để lưu dấu thời gian hiện tại.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu không thành công.
 **********************************************************/
Std_ReturnType CanIf_GetCurrentTime(uint8 Controller, Can_TimeStampType* timeStampPtr);

/**********************************************************
 * @brief Kích hoạt dấu thời gian đầu ra cho một CAN PDU.
 * @param TxPduId ID của PDU truyền.
 **********************************************************/
void CanIf_EnableEgressTimeStamp(PduIdType TxPduId);

/**********************************************************
 * @brief Lấy dấu thời gian đầu ra của một PDU từ phần cứng.
 * @param TxPduId ID của PDU truyền.
 * @param timeStampPtr Con trỏ để lưu dấu thời gian đầu ra.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu không thành công.
 **********************************************************/
Std_ReturnType CanIf_GetEgressTimeStamp(PduIdType TxPduId, Can_TimeStampType* timeStampPtr);

/**********************************************************
 * @brief Lấy dấu thời gian đầu vào của một PDU từ phần cứng.
 * @param RxPduId ID của PDU nhận.
 * @param timeStampPtr Con trỏ để lưu dấu thời gian đầu vào.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu không thành công.
 **********************************************************/
Std_ReturnType CanIf_GetIngressTimeStamp(PduIdType RxPduId, Can_TimeStampType* timeStampPtr);

/**********************************************************
 * @brief Yêu cầu dữ liệu để truyền từ lớp trên.
 * @param TxPduId ID của PDU cần truyền.
 * @param PduInfoPtr Con trỏ đến cấu trúc chứa dữ liệu và metadata của PDU.
 * @return `E_OK` nếu sao chép dữ liệu thành công, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_TriggerTransmit(PduIdType TxPduId, PduInfoType* PduInfoPtr);

/**********************************************************
 * @brief Xác nhận đã truyền thành công một CAN Tx PDU.
 * @param CanTxPduId ID của PDU đã truyền.
 **********************************************************/
void CanIf_TxConfirmation(PduIdType CanTxPduId);

/**********************************************************
 * @brief Báo nhận thành công một CAN Rx PDU cho CanIf sau khi xác thực.
 * @param Mailbox Thông tin về CAN Mailbox.
 * @param PduInfoPtr Con trỏ đến cấu trúc chứa dữ liệu của PDU nhận.
 **********************************************************/
void CanIf_RxIndication(const Can_HwType* Mailbox, const PduInfoType* PduInfoPtr);

/**********************************************************
 * @brief Thông báo sự kiện BusOff của một CAN Controller.
 * @param ControllerId ID của CAN Controller.
 **********************************************************/
void CanIf_ControllerBusOff(uint8 ControllerId);

/**********************************************************
 * @brief Xác nhận rằng Transceiver đang hoạt động ở chế độ truyền thông PN.
 * @param TransceiverId ID của CAN Transceiver.
 **********************************************************/
void CanIf_ConfirmPnAvailability(uint8 TransceiverId);

/**********************************************************
 * @brief Thông báo rằng cờ Wakeup của CAN Transceiver đã được xóa.
 * @param TransceiverId ID của CAN Transceiver.
 **********************************************************/
void CanIf_ClearTrcvWufFlagIndication(uint8 TransceiverId);

/**********************************************************
 * @brief Kiểm tra cờ wake-up của Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @return `E_OK` nếu cờ wake-up được đặt, `E_NOT_OK` nếu không.
 **********************************************************/
Std_ReturnType CanIf_CheckTrcvWakeFlagIndication(uint8 TransceiverId);

/**********************************************************
 * @brief Thông báo sự thay đổi chế độ của một CAN Controller.
 * @param ControllerId ID của CAN Controller.
 * @param ControllerMode Chế độ hiện tại của CAN Controller.
 **********************************************************/
void CanIf_ControllerModeIndication(uint8 ControllerId, Can_ControllerStateType ControllerMode);

/**********************************************************
 * @brief Thông báo sự thay đổi chế độ của một CAN Transceiver.
 * @param TransceiverId ID của CAN Transceiver.
 * @param TransceiverMode Chế độ hiện tại của CAN Transceiver.
 **********************************************************/
void CanIf_TrcvModeIndication(uint8 TransceiverId, CanTrcv_TrcvModeType TransceiverMode);

/**********************************************************
 * @brief Thông báo rằng CAN Controller đã vào trạng thái lỗi thụ động.
 * @param ControllerId ID của CAN Controller.
 **********************************************************/
void CanIf_ControllerErrorStatePassive(uint8 ControllerId);

#endif /* CANIF_H */
