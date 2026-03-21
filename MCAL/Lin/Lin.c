/**********************************************************
 * @file Lin.c
 * @brief AUTOSAR LIN Driver Source File
 * @details File này chứa các định nghĩa hàm cho LIN driver
 *          theo tiêu chuẩn AUTOSAR.
 * @version 1.0
 * @date 2024-11-01
 * @author HALA Academy
 **********************************************************/

#include "Lin.h"
#include "Lin_Cfg.h"

/**********************************************************
 * @brief Khởi tạo mô-đun LIN.
 * @param Config Con trỏ đến cấu trúc cấu hình LIN.
 **********************************************************/
void Lin_Init(const Lin_ConfigType* Config) {
    // Kiểm tra cấu hình hợp lệ
    if (Config == NULL) {
        return;  // Trả về nếu cấu hình không hợp lệ
    }

    // Bật clock cho các port GPIO và UART sử dụng cho LIN
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    // Cấu hình chân Tx (PA9) và Rx (PA10) cho UART sử dụng LIN
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;  // Chân Tx
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  // Chân Rx
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Cấu hình UART cho LIN
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = Config->Lin_BaudRate;  // Tốc độ truyền từ cấu hình
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    // Bật UART
    USART_Cmd(USART1, ENABLE);

    // Bật chế độ LIN
    USART_LINCmd(USART1, ENABLE);

    // Cấu hình ngắt nếu được yêu cầu
    if (Config->Lin_IRQn != 0) {
        NVIC_EnableIRQ(Config->Lin_IRQn);  // Bật ngắt cho LIN
    }
}


/**********************************************************
 * @brief Kiểm tra sự kiện wake-up cho kênh LIN.
 * @param Channel Kênh LIN cần kiểm tra.
 * @return `E_OK` nếu phát hiện sự kiện wake-up, `E_NOT_OK` nếu không phát hiện.
 * @details Hàm này kiểm tra trạng thái của kênh LIN để xác định xem có sự kiện wake-up đã xảy ra hay không.
 **********************************************************/
Std_ReturnType Lin_CheckWakeup(uint8 Channel) {
    // Kiểm tra xem Channel có nằm trong phạm vi hợp lệ
    if (Channel >= MAX_LIN_CHANNELS) {
        return E_NOT_OK;  // Trả về nếu Channel không hợp lệ
    }

    // Đọc trạng thái từ thanh ghi phần cứng hoặc module LIN để kiểm tra sự kiện wake-up
    if (USART1->SR & USART_SR_WU) {  // Giả sử USART_SR_WU là cờ wake-up trong thanh ghi trạng thái USART
        // Xóa cờ wake-up sau khi kiểm tra
        USART1->SR &= ~USART_SR_WU;
        return E_OK;  // Phát hiện wake-up
    }

    return E_NOT_OK;  // Không phát hiện sự kiện wake-up
}


/**********************************************************
 * @brief Lấy thông tin phiên bản của LIN driver.
 * @param versioninfo Con trỏ đến cấu trúc để lưu thông tin phiên bản.
 **********************************************************/
void Lin_GetVersionInfo(Std_VersionInfoType* versioninfo) {
    if (versioninfo != NULL) {
        versioninfo->vendorID = LIN_VENDOR_ID;
        versioninfo->moduleID = LIN_MODULE_ID;
        versioninfo->sw_major_version = LIN_SW_MAJOR_VERSION;
        versioninfo->sw_minor_version = LIN_SW_MINOR_VERSION;
        versioninfo->sw_patch_version = LIN_SW_PATCH_VERSION;
    }
}

/**********************************************************
 * @brief Tính toán giá trị checksum cho một frame LIN.
 * @param data Con trỏ tới mảng dữ liệu cần tính toán checksum.
 * @param length Độ dài của mảng dữ liệu.
 * @return Giá trị checksum tính được.
 * @details Hàm này tính toán checksum theo chuẩn LIN cho các frame.
 **********************************************************/
static uint8 LIN_CalculateChecksum(const uint8* data, uint8 length) {
    uint16 checksum = 0;

    // Cộng tất cả các byte dữ liệu
    for (uint8 i = 0; i < length; i++) {
        checksum += data[i];
        
        // Nếu có bit carry, cộng thêm vào checksum
        if (checksum > 0xFF) {
            checksum = (checksum & 0xFF) + 1;
        }
    }

    // Trả về giá trị bù của checksum
    return (uint8)(~checksum);
}


/**********************************************************
 * @brief Gửi một frame LIN.
 * @param Channel Kênh LIN cần gửi frame.
 * @param PduInfoPtr Con trỏ đến PDU chứa thông tin để gửi.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 **********************************************************/
Std_ReturnType Lin_SendFrame(uint8 Channel, const Lin_PduType* PduInfoPtr) {
    // Kiểm tra tính hợp lệ của tham số đầu vào
    if (PduInfoPtr == NULL) {
        return E_NOT_OK;
    }

    // Bắt đầu gửi khung LIN bằng cách phát trường Break
    USART_SendBreak(USART1);  // Gửi trường Break qua UART

    // Chờ quá trình gửi trường Break hoàn thành
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    // Gửi trường đồng bộ (Sync Field)
    USART_SendData(USART1, 0x55);  // Byte Sync với giá trị cố định 0x55
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    // Tính toán và gửi trường ID
    uint8 id_with_parity = PduInfoPtr->Pid | LIN_CalculateParity(PduInfoPtr->Pid);
    USART_SendData(USART1, id_with_parity);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    // Gửi trường dữ liệu (Data Field)
    for (uint8 i = 0; i < PduInfoPtr->Dl; i++) {
        USART_SendData(USART1, PduInfoPtr->SduPtr[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
    }

    // Tính toán và gửi trường Checksum
    uint8 checksum = LIN_CalculateChecksum(PduInfoPtr->SduPtr, PduInfoPtr->Dl);
    USART_SendData(USART1, checksum);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    return E_OK;  // Trả về `E_OK` nếu quá trình gửi hoàn tất
}


/**********************************************************
 * @brief Chuyển kênh LIN vào chế độ sleep.
 * @param Channel Kênh LIN cần chuyển vào chế độ sleep.
 * @return `E_OK` nếu lệnh sleep được chấp nhận, `E_NOT_OK` nếu xảy ra lỗi.
 * @details Hàm này sẽ gửi lệnh "go-to-sleep" trên kênh LIN chỉ định và
 *          đặt kênh vào trạng thái ngủ để tiết kiệm năng lượng.
 **********************************************************/
Std_ReturnType Lin_GoToSleep(uint8 Channel) {
    // Kiểm tra tính hợp lệ của Channel
    if (Channel >= MAX_LIN_CHANNELS) {
        return E_NOT_OK;  // Channel không hợp lệ
    }

    // Gửi tín hiệu báo "go-to-sleep" bằng cách phát trường Break và gửi khung với ID sleep
    USART_SendBreak(USART1);  // Phát trường Break để báo hiệu sleep

    // Chờ cho quá trình gửi hoàn tất
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    // Đặt trạng thái kênh LIN vào chế độ ngủ
    LinChannelState[Channel] = LIN_CH_SLEEP;

    return E_OK;  // Lệnh sleep được thực hiện thành công
}


/**********************************************************
 * @brief Đặt kênh LIN vào trạng thái sleep và kích hoạt phát hiện wake-up.
 * @param Channel Kênh LIN cần xử lý.
 * @return `E_OK` nếu lệnh được chấp nhận, `E_NOT_OK` nếu xảy ra lỗi.
 * @details Hàm này gửi lệnh "go-to-sleep" trên kênh LIN chỉ định và 
 *          đặt kênh vào chế độ ngủ để tiết kiệm năng lượng. Khi ở chế độ ngủ,
 *          hệ thống sẽ kích hoạt giám sát để phát hiện các tín hiệu wake-up.
 **********************************************************/
Std_ReturnType Lin_GoToSleepInternal(uint8 Channel) {
    // Kiểm tra xem Channel có hợp lệ không
    if (Channel >= MAX_LIN_CHANNELS) {
        return E_NOT_OK;  // Báo lỗi nếu kênh không hợp lệ
    }

    // Gửi tín hiệu "go-to-sleep" bằng cách phát trường Break và gửi ID sleep
    USART_SendBreak(USART1);  // Phát trường Break để báo hiệu chế độ sleep

    // Đợi cho quá trình gửi hoàn tất
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    // Cập nhật trạng thái của kênh LIN thành chế độ sleep
    LinChannelState[Channel] = LIN_CH_SLEEP;

    // Kích hoạt phát hiện wake-up nếu cần
    if (LinChannelConfig[Channel].LinChannelWakeupSupport == ENABLE) {
        // Kích hoạt chế độ phát hiện wake-up, có thể thông qua ngắt hoặc giám sát bus
    }

    return E_OK;  // Trả về `E_OK` nếu quá trình thành công
}


/**********************************************************
 * @brief Phát xung wake-up và đặt trạng thái kênh thành LIN_CH_OPERATIONAL.
 * @param Channel Kênh LIN cần phát xung wake-up.
 * @return `E_OK` nếu thành công, `E_NOT_OK` nếu thất bại.
 * @details Phát tín hiệu wake-up để đánh thức các node LIN từ chế độ sleep.
 **********************************************************/
Std_ReturnType Lin_Wakeup(uint8 Channel) {
    // Kiểm tra tính hợp lệ của Channel
    if (Channel >= MAX_LIN_CHANNELS) {
        return E_NOT_OK;  // Trả về lỗi nếu Channel không hợp lệ
    }

    // Kiểm tra trạng thái của kênh, phải là LIN_CH_SLEEP mới tiếp tục
    if (LinChannelState[Channel] != LIN_CH_SLEEP) {
        return E_NOT_OK;  // Trả về lỗi nếu kênh không ở trạng thái sleep
    }

    // Gửi tín hiệu wake-up bằng cách phát một bit dominant
    USART_SendData(USART1, 0x80);  // Phát byte chứa bit dominant

    // Đợi cho quá trình gửi hoàn tất
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);

    // Cập nhật trạng thái của kênh thành LIN_CH_OPERATIONAL
    LinChannelState[Channel] = LIN_CH_OPERATIONAL;

    return E_OK;  // Trả về `E_OK` nếu thành công
}


/**********************************************************
 * @brief Lấy trạng thái hiện tại của kênh LIN.
 * @param Channel Kênh LIN cần kiểm tra.
 * @param Lin_SduPtr Con trỏ tới một con trỏ chứa SDU hiện tại.
 * @return Trạng thái hiện tại của kênh LIN.
 * @details Hàm này kiểm tra trạng thái của kênh LIN và trả về trạng thái hoạt động hiện tại.
 **********************************************************/
Lin_StatusType Lin_GetStatus(uint8 Channel, const uint8** Lin_SduPtr) {
    // Kiểm tra tính hợp lệ của con trỏ đầu vào
    if (Lin_SduPtr == NULL) {
        return LIN_NOT_OK;  // Trả về lỗi nếu con trỏ không hợp lệ
    }

    // Kiểm tra xem Channel có nằm trong phạm vi hợp lệ hay không
    if (Channel >= MAX_LIN_CHANNELS) {
        return LIN_NOT_OK;  // Trả về lỗi nếu Channel không hợp lệ
    }

    // Lấy trạng thái hiện tại từ phần cứng hoặc từ một biến trạng thái lưu trữ
    Lin_StatusType currentStatus = LinChannelState[Channel];

    // Nếu trạng thái là LIN_RX_OK hoặc LIN_TX_OK, cập nhật Lin_SduPtr
    if (currentStatus == LIN_RX_OK || currentStatus == LIN_TX_OK) {
        *Lin_SduPtr = LinChannelData[Channel];  // LinChannelData là vùng dữ liệu chứa SDU
    } else {
        *Lin_SduPtr = NULL;  // Nếu không có dữ liệu, gán con trỏ SDU là NULL
    }

    return currentStatus;  // Trả về trạng thái hiện tại của kênh LIN
}
