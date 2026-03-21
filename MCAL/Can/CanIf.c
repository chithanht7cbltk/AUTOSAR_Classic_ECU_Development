/**********************************************************
 * @file CanIf.c
 * @brief Implementation file for CAN Interface of AUTOSAR
 * @details File này cung cấp hiện thực các hàm cho CAN Interface, bao gồm
 *          khởi tạo, điều khiển chế độ, và quản lý truyền/nhận dữ liệu
 *          theo tiêu chuẩn AUTOSAR.
 * 
 * @version 1.0
 * @date 2024-10-30
 * @organization HALA Academy
 **********************************************************/

#include "CanIf.h"
#include "Std_Types.h"
#include "CanIf_Types.h"
#include "Can_GeneralTypes.h"

// Định nghĩa biến toàn cục để lưu cấu hình CAN Interface
const CanIf_ConfigType* CanIf_ConfigData = NULL;

// Định nghĩa số lượng tối đa các CAN Controller có trong hệ thống
#define MAX_CAN_CONTROLLERS 2  // hoặc giá trị phù hợp với hệ thống của bạn

Can_ControllerStateType CanIf_ControllerMode[MAX_CAN_CONTROLLERS];

// Định nghĩa mảng lưu trạng thái của các CAN Controller
Can_ControllerStateType CanIf_ControllerStates[MAX_CAN_CONTROLLERS];




/**********************************************************
 * @brief Khởi tạo CAN Interface với cấu hình từ CanIf_ConfigType.
 * @param ConfigPtr Con trỏ đến cấu hình CAN Interface.
 **********************************************************/
void CanIf_Init(const CanIf_ConfigType* ConfigPtr) {
    // Kiểm tra nếu con trỏ cấu hình là NULL
    if (ConfigPtr == NULL) {
        // Nếu con trỏ NULL, không thực hiện khởi tạo
        return;
    }

    // Lưu trữ con trỏ cấu hình vào biến toàn cục để sử dụng cho các hàm khác
    CanIf_ConfigData = ConfigPtr;

    // Khởi tạo trạng thái thông báo và chế độ cho từng PDU dựa trên cấu hình
    for (uint16 i = 0; i < ConfigPtr->NumPdus; i++) {
        // Thiết lập trạng thái thông báo cho mỗi PDU là NO_NOTIFICATION (không có thông báo)
        ConfigPtr->NotifStatus[i] = CANIF_NO_NOTIFICATION;

        // Thiết lập chế độ mặc định cho mỗi PDU
        switch (ConfigPtr->PduConfig[i].PduMode) {
            case CANIF_ONLINE:
                // Chế độ hoạt động bình thường cho cả truyền và nhận
                // Cần cài đặt chế độ cho phần cứng nếu cần
                break;
            case CANIF_TX_OFFLINE:
                // Chế độ chỉ nhận, không truyền
                // Cần cấu hình phần cứng nếu chế độ này yêu cầu
                break;
            case CANIF_TX_OFFLINE_ACTIVE:
                // Chế độ chỉ truyền offline, chỉ cho phép gửi PDU
                break;
            case CANIF_OFFLINE:
                // Chế độ không hoạt động, không cho phép truyền và nhận
                break;
            default:
                // Nếu không đúng các chế độ trên, có thể gán giá trị mặc định là OFFLINE
                ConfigPtr->PduConfig[i].PduMode = CANIF_OFFLINE;
                break;
        }
    }

    // Cấu hình các thông số cho CAN Controller (nếu có)
    // Bao gồm thiết lập chế độ, tốc độ baud và các tính năng của CAN Controller
    for (uint8 controllerId = 0; controllerId < MAX_CAN_CONTROLLERS; controllerId++) {
        if (ConfigPtr->ControllerConfig[controllerId].isConfigured) {
            // Cấu hình tốc độ truyền baud rate nếu được định nghĩa trong cấu hình
            CanIf_SetBaudrate(controllerId, ConfigPtr->ControllerConfig[controllerId].BaudRateConfigID);

            // Thiết lập chế độ hoạt động ban đầu của CAN Controller
            CanIf_SetControllerMode(controllerId, CAN_CS_STOPPED);

            // Khởi tạo chế độ mặc định cho CAN Controller nếu cần
            if (ConfigPtr->ControllerConfig[controllerId].DefaultPduMode != CANIF_OFFLINE) {
                CanIf_SetPduMode(controllerId, ConfigPtr->ControllerConfig[controllerId].DefaultPduMode);
            }
        }
    }

    // Cấu hình bộ đệm truyền và nhận cho CAN Interface (nếu cần)
    // Cấu hình bổ sung cho FIFO, Queue và chế độ ưu tiên của các PDU
    for (uint16 j = 0; j < ConfigPtr->NumPdus; j++) {
        CanIf_PduConfigType* pduConfig = &ConfigPtr->PduConfig[j];
        // Thiết lập bộ đệm, các chế độ ưu tiên cho từng PDU dựa trên cấu hình trong pduConfig
    }

    // Đặt các cờ và trạng thái khác của CAN Interface về giá trị mặc định
    CanIf_InterfaceStatus = CANIF_INITIALIZED;

    // Khởi tạo CAN Interface hoàn tất
}


/**********************************************************
 * @brief Hủy khởi tạo CAN Interface, giải phóng tài nguyên đã sử dụng.
 **********************************************************/
void CanIf_DeInit(void) {
    // Kiểm tra nếu CAN Interface chưa được khởi tạo
    if (CanIf_InterfaceStatus != CANIF_INITIALIZED) {
        return;
    }

    // Đặt lại trạng thái thông báo cho tất cả các PDUs
    for (uint16 i = 0; i < CanIf_ConfigData->NumPdus; i++) {
        CanIf_ConfigData->NotifStatus[i] = CANIF_NO_NOTIFICATION;
    }

    // Đặt tất cả các CAN Controller về chế độ dừng và giải phóng tài nguyên
    for (uint8 controllerId = 0; controllerId < CanIf_ConfigData->NumControllers; controllerId++) {
        if (CanIf_ConfigData->ControllerConfig[controllerId].isConfigured) {
            // Đặt CAN Controller về chế độ STOPPED
            CanIf_SetControllerMode(controllerId, CAN_CS_STOPPED);
        }
    }

    // Giải phóng cấu hình CAN Interface nếu cần
    CanIf_ConfigData = NULL;

    // Đặt trạng thái CAN Interface về chưa khởi tạo
    CanIf_InterfaceStatus = CANIF_UNINITIALIZED;
}


/**********************************************************
 * @brief Thay đổi chế độ hoạt động của một CAN Controller.
 * @param ControllerId ID của CAN controller.
 * @param ControllerMode Chế độ yêu cầu của CAN controller.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_SetControllerMode(uint8 ControllerId, Can_ControllerStateType ControllerMode) {
    // Kiểm tra xem CAN Interface đã được khởi tạo chưa
    if (CanIf_InterfaceStatus != CANIF_INITIALIZED) {
        return E_NOT_OK; // Trả về lỗi nếu CAN Interface chưa được khởi tạo
    }

    // Kiểm tra ID của Controller có hợp lệ không
    if (ControllerId >= MAX_CAN_CONTROLLERS) {
        return E_NOT_OK; // Trả về lỗi nếu ID Controller không hợp lệ
    }

    // Kiểm tra chế độ yêu cầu có hợp lệ không
    if (ControllerMode != CAN_CS_STARTED &&
        ControllerMode != CAN_CS_STOPPED &&
        ControllerMode != CAN_CS_SLEEP) {
        return E_NOT_OK; // Trả về lỗi nếu chế độ không hợp lệ
    }

    // Thay đổi chế độ của CAN Controller dựa trên yêu cầu
    switch (ControllerMode) {
        case CAN_CS_STARTED:
            // Cài đặt CAN Controller vào chế độ STARTED
            // Gọi các hàm khởi tạo hoặc kích hoạt phần cứng cần thiết
            // Ví dụ: Bật ngắt, kích hoạt truyền/nhận
            break;

        case CAN_CS_STOPPED:
            // Đặt CAN Controller vào chế độ STOPPED
            // Vô hiệu hóa ngắt, dừng truyền/nhận
            break;

        case CAN_CS_SLEEP:
            // Đặt CAN Controller vào chế độ SLEEP
            // Giảm tiêu thụ năng lượng, vô hiệu hóa các thành phần không cần thiết
            break;

        default:
            return E_NOT_OK; // Trả về lỗi nếu chế độ không xác định
    }

    // Nếu thay đổi thành công, cập nhật trạng thái hiện tại
    CanIf_ControllerMode[ControllerId] = ControllerMode;

    return E_OK;
}


/**********************************************************
 * @brief Lấy trạng thái hiện tại của CAN Controller.
 * @param ControllerId ID của CAN controller.
 * @param ControllerModePtr Con trỏ lưu trạng thái hiện tại của CAN controller.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetControllerMode(uint8 ControllerId, Can_ControllerStateType* ControllerModePtr) {
    // Kiểm tra nếu con trỏ trạng thái là NULL
    if (ControllerModePtr == NULL) {
        return E_NOT_OK;
    }

    // Kiểm tra tính hợp lệ của ControllerId
    if (ControllerId >= MAX_CAN_CONTROLLERS) {
        return E_NOT_OK;
    }

    // Lấy trạng thái hiện tại của Controller từ hệ thống hoặc phần cứng
    // Giả sử CanIf_ControllerStates là mảng lưu trữ trạng thái của các Controller
    *ControllerModePtr = CanIf_ControllerStates[ControllerId];

    return E_OK;
}


/**********************************************************
 * @brief Nhận trạng thái lỗi của CAN Controller.
 * @param ControllerId ID của CAN controller.
 * @param ErrorStatePtr Con trỏ lưu trạng thái lỗi.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_GetControllerErrorState(uint8 ControllerId, Can_ErrorStateType* ErrorStatePtr) {
    // Kiểm tra nếu con trỏ ErrorStatePtr là NULL
    if (ErrorStatePtr == NULL) {
        return E_NOT_OK;
    }

    // Kiểm tra ID của Controller có hợp lệ hay không
    if (ControllerId >= MAX_CAN_CONTROLLERS) {
        return E_NOT_OK;
    }

    // Giả lập logic để lấy trạng thái lỗi từ phần cứng CAN Controller
    // Ví dụ: Đọc thanh ghi trạng thái lỗi từ phần cứng, sau đó lưu vào ErrorStatePtr
    // Trong trường hợp này, giả lập trạng thái lỗi là CAN_ERRORSTATE_ACTIVE
    *ErrorStatePtr = CAN_ERRORSTATE_ACTIVE; // Giá trị giả lập

    // Kiểm tra nếu việc lấy trạng thái lỗi thành công
    if (*ErrorStatePtr == CAN_ERRORSTATE_ACTIVE || 
        *ErrorStatePtr == CAN_ERRORSTATE_PASSIVE || 
        *ErrorStatePtr == CAN_ERRORSTATE_BUSOFF) {
        return E_OK;
    } else {
        return E_NOT_OK;
    }
}


/**********************************************************
 * @brief Gửi PDU qua CAN Interface.
 * @param TxPduId ID của PDU cần truyền.
 * @param PduInfoPtr Con trỏ đến dữ liệu PDU và metadata.
 * @return `E_OK` nếu truyền thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType CanIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr) {
    // Kiểm tra nếu con trỏ dữ liệu PDU hoặc cấu hình CAN Interface chưa được khởi tạo
    if (PduInfoPtr == NULL || CanIf_ConfigData == NULL) {
        return E_NOT_OK;
    }

    // Kiểm tra ID PDU có hợp lệ hay không
    if (TxPduId >= CanIf_ConfigData->NumPdus) {
        return E_NOT_OK;
    }

    // Kiểm tra nếu dữ liệu PDU rỗng hoặc độ dài không hợp lệ
    if (PduInfoPtr->SduDataPtr == NULL || PduInfoPtr->SduLength == 0) {
        return E_NOT_OK;
    }

    // Gửi PDU qua lớp dưới CAN (ví dụ: CAN Driver)
    // Giả sử có hàm Can_Write trong CAN driver để gửi dữ liệu
    if (Can_Write(TxPduId, PduInfoPtr) != E_OK) {
        return E_NOT_OK;
    }

    // Cập nhật trạng thái thông báo của PDU thành đã truyền
    CanIf_ConfigData->NotifStatus[TxPduId] = CANIF_TX_RX_NOTIFICATION;

    return E_OK;
}


// Tự phát triển tiếp
