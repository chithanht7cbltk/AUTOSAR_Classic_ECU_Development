/**********************************************************
 * @file Can.c
 * @brief CAN Driver Implementation File
 * @details File này triển khai các hàm của CAN driver theo tiêu chuẩn AUTOSAR.
 * @version 1.0
 * @date 2024-10-30 
 * @author HALA Academy
 **********************************************************/
#include "Can.h"
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

/**********************************************************
 * @brief Khởi tạo CAN controller với cấu hình được cung cấp.
 * @param Config Cấu hình CAN driver bao gồm các thông số cơ bản và 
 *        các tính năng bổ sung để điều chỉnh tốc độ, chế độ và các 
 *        chức năng tự động của CAN.
 **********************************************************/
void Can_Init(const Can_ConfigType* Config) {
    CAN_InitTypeDef CAN_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Bước 1: Kích hoạt clock cho CAN1 và GPIOA */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);      // Kích hoạt clock cho CAN1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);     // Kích hoạt clock cho GPIOA cho chân CAN

    /* Bước 2: Cấu hình chân CAN RX (PA11) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;                // Chọn chân PA11 cho CAN RX
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;             // Thiết lập chế độ input với pull-up
    GPIO_Init(GPIOA, &GPIO_InitStructure);                    // Khởi tạo PA11 với cấu hình trên
    
    /* Bước 3: Cấu hình chân CAN TX (PA12) */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;                // Chọn chân PA12 cho CAN TX
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         // Tốc độ chân TX ở mức 50MHz
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;           // Chế độ Alternate Function Push-Pull
    GPIO_Init(GPIOA, &GPIO_InitStructure);                    // Khởi tạo PA12 với cấu hình trên

    /* Bước 4: Thiết lập các thông số cơ bản cho CAN_InitTypeDef */
    CAN_InitStructure.CAN_Prescaler = Config->BasicConfig.CAN_Prescaler; // Thiết lập giá trị chia tần số
    CAN_InitStructure.CAN_Mode = Config->BasicConfig.CAN_Mode;           // Chế độ CAN: Normal, Loopback, Silent
    CAN_InitStructure.CAN_SJW = Config->BasicConfig.CAN_SJW;             // Độ rộng của khoảng nhảy đồng bộ
    CAN_InitStructure.CAN_BS1 = Config->BasicConfig.CAN_BS1;             // Độ dài của segment 1
    CAN_InitStructure.CAN_BS2 = Config->BasicConfig.CAN_BS2;             // Độ dài của segment 2

    /* Bước 5: Thiết lập các tính năng bổ sung cho CAN */
    CAN_InitStructure.CAN_TTCM = Config->Features.CAN_TTCM;             // Kích hoạt Time Triggered Communication Mode
    CAN_InitStructure.CAN_ABOM = Config->Features.CAN_ABOM;             // Kích hoạt Automatic Bus-Off Management
    CAN_InitStructure.CAN_AWUM = Config->Features.CAN_AWUM;             // Kích hoạt Automatic Wake-Up Mode
    CAN_InitStructure.CAN_NART = Config->Features.CAN_NART;             // Kích hoạt Non-Automatic Retransmission
    CAN_InitStructure.CAN_RFLM = Config->Features.CAN_RFLM;             // Kích hoạt Receive FIFO Locked Mode
    CAN_InitStructure.CAN_TXFP = Config->Features.CAN_TXFP;             // Kích hoạt Transmit FIFO Priority
    
    /* Bước 6: Khởi tạo CAN1 với cấu hình CAN_InitStructure */
    if (CAN_Init(CAN1, &CAN_InitStructure) != CAN_InitStatus_Success) {
        // Xử lý lỗi nếu việc khởi tạo CAN không thành công
        // Có thể ghi lại log hoặc thông báo lỗi tại đây
    }
}



/**********************************************************
 * @brief Lấy thông tin phiên bản của CAN driver.
 * @param versioninfo Con trỏ đến struct để lưu thông tin phiên bản.
 **********************************************************/
void Can_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    if (versioninfo != NULL) {
        versioninfo->vendorID = 0x1234;
        versioninfo->moduleID = 0x01;
        versioninfo->sw_major_version = 1;
        versioninfo->sw_minor_version = 0;
        versioninfo->sw_patch_version = 0;
    }
}

/**********************************************************
 * @brief Hủy khởi tạo CAN driver.
 * @details Hàm này tắt CAN controller, giải phóng các tài nguyên đã được cấp phát, 
 *          và vô hiệu hóa các cấu hình liên quan đến CAN để tiết kiệm tài nguyên 
 *          và đảm bảo CAN controller không còn hoạt động.
 **********************************************************/
void Can_DeInit(void) {
    /* Bước 1: Vô hiệu hóa CAN controller */
    CAN_Cmd(CAN1, DISABLE);                         // Tắt CAN1, dừng mọi hoạt động của CAN controller

    /* Bước 2: Reset các thanh ghi của CAN1 để đưa về trạng thái mặc định */
    CAN_DeInit(CAN1);                               // Đặt lại toàn bộ cấu hình của CAN1 về mặc định
    
    /* Bước 3: Vô hiệu hóa ngắt CAN nếu đã được kích hoạt */
    CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_TME | CAN_IT_ERR, DISABLE);  // Tắt toàn bộ ngắt liên quan đến CAN1
    
    /* Bước 4: Tắt clock của CAN1 và GPIOA để tiết kiệm năng lượng */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, DISABLE);   // Tắt clock cho CAN1
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, DISABLE);  // Tắt clock cho GPIOA nếu không còn sử dụng

    /* Bước 5: Xóa cấu hình các chân CAN TX (PA12) và RX (PA11) về mặc định */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;           // Đặt lại về chế độ Analog để tiết kiệm năng lượng
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


/**********************************************************
 * @brief Thiết lập tốc độ truyền cho một CAN controller.
 * @param Controller ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param BaudRateConfigID ID cấu hình tốc độ truyền, xác định tốc độ 
 *        baud mong muốn.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 * @details Hàm này sử dụng `BaudRateConfigID` để điều chỉnh tốc độ truyền
 *          của CAN controller. Giá trị BaudRateConfigID có thể tương ứng
 *          với các tốc độ như 125 kbps, 250 kbps, 500 kbps, v.v.
 **********************************************************/
Std_ReturnType Can_SetBaudrate(uint8 Controller, uint16 BaudRateConfigID) {
    /* Kiểm tra xem Controller có hợp lệ không */
    if (Controller != 0) { // Giả định Controller 0 là CAN1
        return E_NOT_OK; // Chỉ hỗ trợ CAN1 trong ví dụ này
    }
    
    /* Tắt CAN1 trước khi cấu hình lại baudrate */
    CAN_Cmd(CAN1, DISABLE);

    /* Cấu hình CAN_InitTypeDef để đặt lại thông số CAN */
    CAN_InitTypeDef CAN_InitStructure;
    CAN_StructInit(&CAN_InitStructure);  // Đặt cấu hình mặc định
    
    /* Xác định tốc độ baud dựa trên BaudRateConfigID */
    switch (BaudRateConfigID) {
        case 125: // 125 kbps
            CAN_InitStructure.CAN_Prescaler = 48;  // Thiết lập prescaler cho 125 kbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_14tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;
        
        case 250: // 250 kbps
            CAN_InitStructure.CAN_Prescaler = 24;  // Thiết lập prescaler cho 250 kbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_14tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;
        
        case 500: // 500 kbps
            CAN_InitStructure.CAN_Prescaler = 12;  // Thiết lập prescaler cho 500 kbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_14tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;

        case 1000: // 1 Mbps
            CAN_InitStructure.CAN_Prescaler = 6;   // Thiết lập prescaler cho 1 Mbps
            CAN_InitStructure.CAN_BS1 = CAN_BS1_14tq;
            CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;
            CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;
            break;

        default:
            return E_NOT_OK; // Nếu BaudRateConfigID không hợp lệ
    }
    
    /* Khởi tạo lại CAN1 với cấu hình mới */
    if (CAN_Init(CAN1, &CAN_InitStructure) != CAN_InitStatus_Success) {
        return E_NOT_OK; // Khởi tạo thất bại
    }

    /* Kích hoạt lại CAN1 sau khi cấu hình */
    CAN_Cmd(CAN1, ENABLE);

    return E_OK; // Thiết lập thành công
}


/**********************************************************
 * @brief Chuyển đổi trạng thái của một CAN controller.
 * @param Controller ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param Transition Trạng thái chuyển đổi mong muốn:
 *        - CAN_CS_STARTED: Bắt đầu hoạt động của CAN.
 *        - CAN_CS_STOPPED: Dừng hoạt động của CAN.
 *        - CAN_CS_SLEEP: Đưa CAN vào chế độ sleep.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 * @details Hàm này cho phép chuyển đổi trạng thái của CAN controller
 *          giữa các trạng thái STARTED, STOPPED, và SLEEP, đảm bảo 
 *          CAN hoạt động phù hợp với yêu cầu hệ thống.
 **********************************************************/
Std_ReturnType Can_SetControllerMode(uint8 Controller, Can_ControllerStateType Transition) {
    /* Kiểm tra xem Controller có hợp lệ không */
    if (Controller != 0) {  // Giả định Controller 0 là CAN1
        return E_NOT_OK;    // Chỉ hỗ trợ CAN1 trong ví dụ này
    }

    /* Thực hiện chuyển đổi trạng thái dựa trên Transition */
    switch (Transition) {
        case CAN_CS_STARTED:
            CAN_Cmd(CAN1, ENABLE);  // Bắt đầu hoạt động của CAN1
            break;
        
        case CAN_CS_STOPPED:
            CAN_Cmd(CAN1, DISABLE); // Dừng hoạt động của CAN1
            break;
        
        case CAN_CS_SLEEP:
            /* Kích hoạt chế độ sleep */
            CAN1->MCR |= CAN_MCR_SLEEP;  // Đặt bit SLEEP trong MCR để chuyển sang chế độ sleep
            if (!(CAN1->MSR & CAN_MSR_SLAK)) {
                return E_NOT_OK;         // Kiểm tra lại bit SLAK để xác nhận chế độ sleep
            }
            break;

        default:
            return E_NOT_OK; // Nếu Transition không hợp lệ
    }

    /* Xác nhận trạng thái chuyển đổi */
    if (Transition == CAN_CS_STARTED) {
        if (!(CAN1->MSR & CAN_MSR_INAK)) {
            return E_OK;  // CAN đã ở trạng thái hoạt động
        } else {
            return E_NOT_OK; // Nếu bit INAK không tắt, chuyển đổi thất bại
        }
    } else if (Transition == CAN_CS_STOPPED) {
        if (CAN1->MSR & CAN_MSR_INAK) {
            return E_OK;  // CAN đã ở trạng thái dừng
        } else {
            return E_NOT_OK; // Nếu bit INAK không được bật, chuyển đổi thất bại
        }
    }

    return E_OK;  // Trạng thái chuyển đổi thành công
}

/**********************************************************
 * @brief Vô hiệu hóa ngắt cho một CAN controller.
 * @param Controller ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @details Hàm này vô hiệu hóa tất cả các ngắt liên quan đến CAN controller
 *          để đảm bảo rằng không có sự gián đoạn nào xảy ra trong quá trình
 *          xử lý hoặc cấu hình lại CAN. Điều này thường được thực hiện trước
 *          khi dừng hoặc cấu hình lại CAN.
 **********************************************************/
void Can_DisableControllerInterrupts(uint8 Controller) {
    /* Kiểm tra xem Controller có hợp lệ không */
    if (Controller != 0) {  // Giả định Controller 0 là CAN1
        return;             // Chỉ hỗ trợ CAN1 trong ví dụ này
    }

    /* Vô hiệu hóa tất cả các ngắt của CAN1 */
    CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);   // Vô hiệu hóa ngắt truyền
    CAN_ITConfig(CAN1, CAN_IT_FMP0, DISABLE);  // Vô hiệu hóa ngắt FIFO 0
    CAN_ITConfig(CAN1, CAN_IT_FMP1, DISABLE);  // Vô hiệu hóa ngắt FIFO 1
    CAN_ITConfig(CAN1, CAN_IT_ERR, DISABLE);   // Vô hiệu hóa ngắt lỗi
    CAN_ITConfig(CAN1, CAN_IT_WKU, DISABLE);   // Vô hiệu hóa ngắt wake-up
    CAN_ITConfig(CAN1, CAN_IT_SLK, DISABLE);   // Vô hiệu hóa ngắt sleep

    /* Xóa các cờ ngắt để đảm bảo không có ngắt nào được kích hoạt sau khi vô hiệu hóa */
    CAN_ClearITPendingBit(CAN1, CAN_IT_TME);   // Xóa cờ ngắt truyền
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP0);  // Xóa cờ ngắt FIFO 0
    CAN_ClearITPendingBit(CAN1, CAN_IT_FMP1);  // Xóa cờ ngắt FIFO 1
    CAN_ClearITPendingBit(CAN1, CAN_IT_ERR);   // Xóa cờ ngắt lỗi
    CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);   // Xóa cờ ngắt wake-up
    CAN_ClearITPendingBit(CAN1, CAN_IT_SLK);   // Xóa cờ ngắt sleep
}


/**********************************************************
 * @brief Kích hoạt ngắt cho một CAN controller.
 * @param Controller ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @details Hàm này kích hoạt tất cả các ngắt liên quan đến CAN controller,
 *          bao gồm ngắt truyền, ngắt FIFO và ngắt lỗi, để đảm bảo rằng các 
 *          sự kiện liên quan đến CAN sẽ được xử lý kịp thời khi chúng xảy ra.
 **********************************************************/
void Can_EnableControllerInterrupts(uint8 Controller) {
    /* Kiểm tra xem Controller có hợp lệ không */
    if (Controller != 0) {  // Giả định Controller 0 là CAN1
        return;             // Chỉ hỗ trợ CAN1 trong ví dụ này
    }

    /* Kích hoạt tất cả các ngắt của CAN1 */
    CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);   // Kích hoạt ngắt truyền
    CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);  // Kích hoạt ngắt FIFO 0
    CAN_ITConfig(CAN1, CAN_IT_FMP1, ENABLE);  // Kích hoạt ngắt FIFO 1
    CAN_ITConfig(CAN1, CAN_IT_ERR, ENABLE);   // Kích hoạt ngắt lỗi
    CAN_ITConfig(CAN1, CAN_IT_WKU, ENABLE);   // Kích hoạt ngắt wake-up
    CAN_ITConfig(CAN1, CAN_IT_SLK, ENABLE);   // Kích hoạt ngắt sleep
}


/**********************************************************
 * @brief Kiểm tra sự kiện đánh thức cho một CAN controller.
 * @param Controller ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @return `E_OK` nếu thành công (controller đã được đánh thức), 
 *         `E_NOT_OK` nếu không có sự kiện đánh thức.
 * @details Hàm này kiểm tra bit wake-up trong thanh ghi CAN để xác định 
 *          xem controller có nhận được sự kiện wake-up từ chế độ sleep không.
 **********************************************************/
Std_ReturnType Can_CheckWakeup(uint8 Controller) {
    /* Kiểm tra xem Controller có hợp lệ không */
    if (Controller != 0) {  // Giả định Controller 0 là CAN1
        return E_NOT_OK;    // Chỉ hỗ trợ CAN1 trong ví dụ này
    }

    /* Kiểm tra bit wake-up trong thanh ghi CAN_MSR */
    if (CAN_GetITStatus(CAN1, CAN_IT_WKU) != RESET) { // Kiểm tra cờ wake-up
        CAN_ClearITPendingBit(CAN1, CAN_IT_WKU);       // Xóa cờ wake-up sau khi xác nhận
        return E_OK;                                   // CAN controller đã được đánh thức
    }

    return E_NOT_OK;  // Không có sự kiện đánh thức xảy ra
}

/**********************************************************
 * @brief Lấy trạng thái lỗi của một CAN controller.
 * @param ControllerId ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param ErrorStatePtr Con trỏ để lưu trạng thái lỗi hiện tại của CAN.
 * @return `E_OK` nếu thành công (trạng thái lỗi được lưu vào ErrorStatePtr), 
 *         `E_NOT_OK` nếu thất bại hoặc controller không hợp lệ.
 * @details Hàm này đọc trạng thái lỗi của CAN controller từ các thanh ghi lỗi.
 *          Trạng thái lỗi có thể là CAN_ERRORSTATE_ACTIVE, CAN_ERRORSTATE_PASSIVE, 
 *          hoặc CAN_ERRORSTATE_BUS_OFF, dựa trên tình trạng của CAN.
 **********************************************************/
Std_ReturnType Can_GetControllerErrorState(uint8 ControllerId, Can_ErrorStateType* ErrorStatePtr) {
    /* Kiểm tra Controller ID có hợp lệ và con trỏ ErrorStatePtr có null không */
    if (ControllerId != 0 || ErrorStatePtr == NULL) {  // Giả định Controller ID 0 là CAN1
        return E_NOT_OK;  // Chỉ hỗ trợ CAN1 và kiểm tra con trỏ hợp lệ
    }

    /* Đọc trạng thái lỗi từ thanh ghi ESR (Error Status Register) */
    uint32_t errorStatus = CAN1->ESR;

    /* Xác định trạng thái lỗi dựa trên các bit trong ESR */
    if (errorStatus & CAN_ESR_BOFF) {
        *ErrorStatePtr = CAN_ERRORSTATE_BUS_OFF;  // CAN đang ở trạng thái Bus-Off
    } else if (errorStatus & CAN_ESR_EPVF) {
        *ErrorStatePtr = CAN_ERRORSTATE_PASSIVE;  // CAN đang ở trạng thái Error Passive
    } else {
        *ErrorStatePtr = CAN_ERRORSTATE_ACTIVE;   // CAN đang ở trạng thái hoạt động bình thường
    }

    return E_OK;  // Lấy trạng thái lỗi thành công
}


/**********************************************************
 * @brief Lấy trạng thái hiện tại của một CAN controller.
 * @param Controller ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param ControllerModePtr Con trỏ để lưu trạng thái của controller.
 * @return `E_OK` nếu thành công (trạng thái được lưu vào ControllerModePtr), 
 *         `E_NOT_OK` nếu thất bại hoặc controller không hợp lệ.
 * @details Hàm này đọc trạng thái hiện tại của CAN controller từ các thanh ghi điều khiển.
 *          Trạng thái có thể là CAN_CS_STARTED, CAN_CS_STOPPED, hoặc CAN_CS_SLEEP,
 *          dựa trên tình trạng của CAN.
 **********************************************************/
Std_ReturnType Can_GetControllerMode(uint8 Controller, Can_ControllerStateType* ControllerModePtr) {
    /* Kiểm tra Controller ID có hợp lệ và con trỏ ControllerModePtr có null không */
    if (Controller != 0 || ControllerModePtr == NULL) {  // Giả định Controller ID 0 là CAN1
        return E_NOT_OK;  // Chỉ hỗ trợ CAN1 và kiểm tra con trỏ hợp lệ
    }

    /* Đọc trạng thái hiện tại từ thanh ghi MSR (Master Status Register) */
    uint32_t controllerStatus = CAN1->MSR;

    /* Xác định trạng thái hiện tại dựa trên các bit trong MSR */
    if (controllerStatus & CAN_MSR_INAK) {
        *ControllerModePtr = CAN_CS_STOPPED;  // CAN đang ở trạng thái Stopped (Initialization Acknowledge)
    } else if (controllerStatus & CAN_MSR_SLAK) {
        *ControllerModePtr = CAN_CS_SLEEP;    // CAN đang ở trạng thái Sleep (Sleep Acknowledge)
    } else {
        *ControllerModePtr = CAN_CS_STARTED;  // CAN đang ở trạng thái Started (hoạt động bình thường)
    }

    return E_OK;  // Lấy trạng thái thành công
}


/**********************************************************
 * @brief Lấy bộ đếm lỗi Rx của một CAN controller.
 * @param ControllerId ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param RxErrorCounterPtr Con trỏ để lưu giá trị bộ đếm lỗi Rx.
 * @return `E_OK` nếu thành công (giá trị lỗi Rx được lưu vào RxErrorCounterPtr), 
 *         `E_NOT_OK` nếu thất bại hoặc controller không hợp lệ.
 * @details Hàm này đọc giá trị bộ đếm lỗi nhận của CAN controller từ thanh ghi
 *          ECR (Error Counter Register), cho biết số lượng lỗi xảy ra trong quá trình nhận.
 **********************************************************/
Std_ReturnType Can_GetControllerRxErrorCounter(uint8 ControllerId, uint8* RxErrorCounterPtr) {
    /* Kiểm tra Controller ID có hợp lệ và con trỏ RxErrorCounterPtr có null không */
    if (ControllerId != 0 || RxErrorCounterPtr == NULL) {  // Giả định Controller ID 0 là CAN1
        return E_NOT_OK;  // Chỉ hỗ trợ CAN1 và kiểm tra con trỏ hợp lệ
    }

    /* Đọc bộ đếm lỗi Rx từ thanh ghi ESR (Error Status Register) */
    *RxErrorCounterPtr = (uint8)((CAN1->ESR & CAN_ESR_REC) >> 24);  // Giá trị Rx Error Counter nằm ở các bit [31:24]

    return E_OK;  // Lấy bộ đếm lỗi Rx thành công
}


/**********************************************************
 * @brief Lấy bộ đếm lỗi Tx của một CAN controller.
 * @param ControllerId ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param TxErrorCounterPtr Con trỏ để lưu giá trị bộ đếm lỗi Tx.
 * @return `E_OK` nếu thành công (giá trị lỗi Tx được lưu vào TxErrorCounterPtr), 
 *         `E_NOT_OK` nếu thất bại hoặc controller không hợp lệ.
 * @details Hàm này đọc giá trị bộ đếm lỗi truyền của CAN controller từ thanh ghi
 *          ECR (Error Counter Register), cho biết số lượng lỗi xảy ra trong quá trình truyền.
 **********************************************************/
Std_ReturnType Can_GetControllerTxErrorCounter(uint8 ControllerId, uint8* TxErrorCounterPtr) {
    /* Kiểm tra Controller ID có hợp lệ và con trỏ TxErrorCounterPtr có null không */
    if (ControllerId != 0 || TxErrorCounterPtr == NULL) {  // Giả định Controller ID 0 là CAN1
        return E_NOT_OK;  // Chỉ hỗ trợ CAN1 và kiểm tra con trỏ hợp lệ
    }

    /* Đọc bộ đếm lỗi Tx từ thanh ghi ESR (Error Status Register) */
    *TxErrorCounterPtr = (uint8)((CAN1->ESR & CAN_ESR_TEC) >> 16);  // Giá trị Tx Error Counter nằm ở các bit [23:16]

    return E_OK;  // Lấy bộ đếm lỗi Tx thành công
}


/**********************************************************
 * @brief Lấy thời gian hiện tại từ thanh ghi phần cứng của CAN controller.
 * @param ControllerId ID của controller (chỉ hỗ trợ CAN1 trong ví dụ này).
 * @param timeStampPtr Con trỏ để lưu giá trị thời gian hiện tại.
 * @return `E_OK` nếu thành công (thời gian hiện tại được lưu vào timeStampPtr), 
 *         `E_NOT_OK` nếu thất bại hoặc controller không hợp lệ.
 * @details Hàm này đọc giá trị thời gian hiện tại của CAN controller từ thanh ghi 
 *          có sẵn hoặc tính toán thời gian dựa trên các thông số hiện tại.
 *          Trong ví dụ này, thời gian được đọc từ thanh ghi giả định.
 **********************************************************/
Std_ReturnType Can_GetCurrentTime(uint8 ControllerId, Can_TimeStampType* timeStampPtr) {
    /* Kiểm tra Controller ID có hợp lệ và con trỏ timeStampPtr có null không */
    if (ControllerId != 0 || timeStampPtr == NULL) {  // Giả định Controller ID 0 là CAN1
        return E_NOT_OK;  // Chỉ hỗ trợ CAN1 và kiểm tra con trỏ hợp lệ
    }

    /* Giả định thanh ghi thời gian hiện tại có sẵn trên phần cứng CAN */
    uint32_t currentTime = CAN1->TIME;  // Ví dụ giả định CAN1->TIME chứa thời gian hiện tại

    /* Phân tách thời gian thành giây và nano giây (giả định) */
    timeStampPtr->seconds = (currentTime / 1000000000U);    // Giả định tính giây từ currentTime
    timeStampPtr->nanoseconds = (currentTime % 1000000000U); // Lấy phần nano giây còn lại

    return E_OK;  // Lấy thời gian hiện tại thành công
}


/**********************************************************
 * @brief Kích hoạt đánh dấu thời gian đầu ra cho một CAN handle.
 * @param Hth CAN handle cho việc truyền dữ liệu.
 * @details Hàm này kích hoạt tính năng đánh dấu thời gian cho các gói dữ liệu
 *          truyền đi từ CAN handle (Hth), cho phép ghi lại thời gian chính xác
 *          của mỗi gói dữ liệu khi nó được truyền ra khỏi CAN controller.
 **********************************************************/
void Can_EnableEgressTimeStamp(Can_HwHandleType Hth) {
    /* Kiểm tra xem Hth có hợp lệ không */
    if (Hth >= CAN_MAX_HANDLES) {
        return;  // Không thực hiện gì nếu Hth nằm ngoài phạm vi hợp lệ
    }

    /* Kích hoạt timestamp cho CAN handle cụ thể */
    switch (Hth) {
        case 0:
            CAN1->MCR |= CAN_MCR_TTCM;  // Kích hoạt Time Triggered Communication Mode cho CAN handle 0
            break;
        
        case 1:
            CAN1->MCR |= CAN_MCR_TTCM;  // Kích hoạt Time Triggered Communication Mode cho CAN handle 1
            break;

        /* Bổ sung thêm các trường hợp nếu có nhiều CAN handles */
        
        default:
            return;  // Không thực hiện gì nếu Hth không được hỗ trợ
    }
}

/**********************************************************
 * @brief Lấy thời gian đầu ra cho một PDU.
 * @param TxPduId ID của PDU truyền.
 * @param Hth CAN handle cho việc truyền.
 * @param timeStampPtr Con trỏ để lưu thời gian đầu ra.
 * @return `E_OK` nếu thành công (thời gian đầu ra được lưu vào timeStampPtr),
 *         `E_NOT_OK` nếu thất bại hoặc không có timestamp cho PDU.
 * @details Hàm này truy xuất thời gian đầu ra của một gói CAN dựa trên
 *          CAN handle (Hth) và PDU ID (TxPduId), cho phép lấy timestamp 
 *          của gói đã truyền để hỗ trợ các chức năng time-triggered.
 **********************************************************/
Std_ReturnType Can_GetEgressTimeStamp(PduIdType TxPduId, Can_HwHandleType Hth, Can_TimeStampType* timeStampPtr) {
    /* Kiểm tra tính hợp lệ của TxPduId, Hth và con trỏ timeStampPtr */
    if (Hth >= CAN_MAX_HANDLES || timeStampPtr == NULL) {
        return E_NOT_OK;  // Trả về E_NOT_OK nếu CAN handle hoặc con trỏ không hợp lệ
    }

    /* Kiểm tra nếu CAN handle (Hth) có hỗ trợ timestamp */
    if (!(CAN1->MCR & CAN_MCR_TTCM)) {
        return E_NOT_OK;  // Trả về E_NOT_OK nếu Time Triggered Mode không được kích hoạt
    }

    /* Giả định có thanh ghi chứa timestamp cho CAN handle (ví dụ: CAN1->TST) */
    uint32 egressTime = CAN1->TST;  // Đọc thời gian đầu ra từ thanh ghi TST (giả định)

    /* Phân tách thời gian thành giây và nano giây (giả định) */
    timeStampPtr->seconds = (egressTime / 1000000000U);    // Tính giây từ giá trị thời gian
    timeStampPtr->nanoseconds = (egressTime % 1000000000U); // Lấy phần nano giây còn lại

    return E_OK;  // Lấy thời gian đầu ra thành công
}


/**********************************************************
 * @brief Lấy thời gian đầu vào cho một CAN handle.
 * @param Hrh CAN handle cho việc nhận dữ liệu.
 * @param timeStampPtr Con trỏ để lưu thời gian đầu vào.
 * @return `E_OK` nếu thành công (thời gian đầu vào được lưu vào timeStampPtr),
 *         `E_NOT_OK` nếu thất bại hoặc không có timestamp cho dữ liệu nhận.
 * @details Hàm này truy xuất thời gian đầu vào của một gói CAN nhận được dựa trên
 *          CAN handle (Hrh), cho phép lấy timestamp của gói đã nhận để hỗ trợ các
 *          chức năng time-triggered.
 **********************************************************/
Std_ReturnType Can_GetIngressTimeStamp(Can_HwHandleType Hrh, Can_TimeStampType* timeStampPtr) {
    /* Kiểm tra tính hợp lệ của Hrh và con trỏ timeStampPtr */
    if (Hrh >= CAN_MAX_HANDLES || timeStampPtr == NULL) {
        return E_NOT_OK;  // Trả về E_NOT_OK nếu CAN handle hoặc con trỏ không hợp lệ
    }

    /* Kiểm tra nếu CAN handle (Hrh) có hỗ trợ timestamp */
    if (!(CAN1->MCR & CAN_MCR_TTCM)) {
        return E_NOT_OK;  // Trả về E_NOT_OK nếu Time Triggered Mode không được kích hoạt
    }

    /* Giả định có thanh ghi chứa timestamp cho dữ liệu nhận (ví dụ: CAN1->IST) */
    uint32_t ingressTime = CAN1->IST;  // Đọc thời gian đầu vào từ thanh ghi IST (giả định)

    /* Phân tách thời gian thành giây và nano giây (giả định) */
    timeStampPtr->seconds = (ingressTime / 1000000000U);    // Tính giây từ giá trị thời gian
    timeStampPtr->nanoseconds = (ingressTime % 1000000000U); // Lấy phần nano giây còn lại

    return E_OK;  // Lấy thời gian đầu vào thành công
}


