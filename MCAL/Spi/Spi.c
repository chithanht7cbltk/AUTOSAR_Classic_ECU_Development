/**********************************************************
 * @file Spi.c
 * @brief Định nghĩa các hàm liên quan đến giao tiếp SPI
 * @details File này chứa phần định nghĩa của các hàm đã khai báo trong Spi.h,
 *          thực hiện các chức năng khởi tạo, truyền dữ liệu và quản lý trạng thái cho giao tiếp SPI.
 * @version 1.0
 * @date 2024-10-01
 * @author HALA Academy
 **********************************************************/

#include "Spi.h"

// Các biến toàn cục để quản lý trạng thái SPI
static Spi_StatusType SpiStatus = SPI_UNINIT;
static Spi_JobResultType JobResult = SPI_JOB_OK;
static Spi_SeqResultType SeqResult = SPI_SEQ_OK;

/**********************************************************
 * @brief Khởi tạo SPI Handler/Driver với cấu hình đã cho
 * @param[in] ConfigPtr Con trỏ tới cấu trúc cấu hình Spi_ConfigType
 * @details Khởi tạo các thành phần của SPI, bao gồm các kênh và phần cứng SPI.
 **********************************************************/
void Spi_Init(const Spi_ConfigType* ConfigPtr) {
    // Kiểm tra nếu ConfigPtr là NULL
    if (ConfigPtr == NULL) {
        return;  // Không khởi tạo nếu không có cấu hình
    }

    // Thiết lập các giá trị mặc định nếu chưa được cấu hình
    Spi_SetupDefaultConfig((Spi_ConfigType*)ConfigPtr);

    // Khởi tạo và kích hoạt phần cứng SPI1 hoặc SPI2
    if (ConfigPtr->Channel == SPI_CHANNEL_1) {
        // Khởi tạo SPI1 với cấu hình từ ConfigPtr
        Spi_Hw_Init_SPI1(ConfigPtr);
        // Bật SPI1 sau khi khởi tạo
        Spi_Hw_Enable_SPI1();
    } else if (ConfigPtr->Channel == SPI_CHANNEL_2) {
        // Khởi tạo SPI2 với cấu hình từ ConfigPtr
        Spi_Hw_Init_SPI2(ConfigPtr);
        // Bật SPI2 sau khi khởi tạo
        Spi_Hw_Enable_SPI2();
    } else {
        // Xử lý các kênh SPI khác nếu cần
        return;  // Không thực hiện nếu không phải SPI1 hoặc SPI2
    }

    // Cập nhật trạng thái SPI sau khi khởi tạo thành công
    SpiStatus = SPI_IDLE;         // SPI đã sẵn sàng và ở trạng thái nhàn rỗi
    JobResult = SPI_JOB_OK;       // Thiết lập kết quả Job về trạng thái mặc định
    SeqResult = SPI_SEQ_OK;       // Thiết lập kết quả Sequence về trạng thái mặc định
}

/**********************************************************
 * @brief Hủy khởi tạo SPI Handler/Driver
 * @return E_OK nếu thành công, E_NOT_OK nếu lỗi
 * @details Hủy khởi tạo và giải phóng tài nguyên liên quan đến SPI.
 **********************************************************/
Std_ReturnType Spi_DeInit(void) {
    // Kiểm tra trạng thái của SPI trước khi hủy khởi tạo
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo, không thể hủy
    }

    // Hủy khởi tạo phần cứng SPI1
    Spi_Hw_DeInit_SPI1();

    // Hủy khởi tạo phần cứng SPI2
    Spi_Hw_DeInit_SPI2();

    // Đặt trạng thái SPI về trạng thái chưa khởi tạo
    SpiStatus = SPI_UNINIT;

    // Cập nhật kết quả Job và Sequence về trạng thái mặc định
    JobResult = SPI_JOB_OK;
    SeqResult = SPI_SEQ_OK;

    return E_OK;  // Hủy khởi tạo thành công
}

/**********************************************************
 * @brief Ghi dữ liệu vào bộ đệm nội bộ của SPI
 * @param[in] Channel ID của kênh SPI
 * @param[in] DataBufferPtr Con trỏ tới buffer chứa dữ liệu cần ghi
 * @return E_OK nếu ghi thành công, E_NOT_OK nếu có lỗi
 **********************************************************/
Std_ReturnType Spi_WriteIB(Spi_ChannelType Channel, const Spi_DataBufferType* DataBufferPtr) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Kiểm tra nếu buffer dữ liệu là NULL
    if (DataBufferPtr == NULL) {
        return E_NOT_OK;  // Dữ liệu không hợp lệ
    }

    // Kiểm tra kênh SPI (SPI1 hoặc SPI2) và thực hiện ghi dữ liệu tương ứng
    if (Channel == SPI_CHANNEL_1) {
        // Đợi cho đến khi bộ đệm truyền của SPI1 trống
        Spi_Hw_WaitTransmitBufferEmpty_SPI1();

        // Gửi dữ liệu qua SPI1
        Spi_I2S_SendData_SPI1(DataBufferPtr);
    } else if (Channel == SPI_CHANNEL_2) {
        // Đợi cho đến khi bộ đệm truyền của SPI2 trống
        Spi_Hw_WaitTransmitBufferEmpty_SPI2();

        // Gửi dữ liệu qua SPI2
        Spi_I2S_SendData_SPI2(DataBufferPtr);
    } else {
        // Nếu không phải SPI1 hoặc SPI2, trả về lỗi
        return E_NOT_OK;
    }

    return E_OK;  // Ghi dữ liệu thành công
}

/**********************************************************
 * @brief Truyền dữ liệu không đồng bộ qua SPI
 * @param[in] Sequence ID của Sequence cần truyền
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Truyền dữ liệu không đồng bộ qua các Job trong Sequence SPI.
 **********************************************************/
Std_ReturnType Spi_AsyncTransmit(Spi_SequenceType Sequence) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Kiểm tra nếu Sequence vượt quá giới hạn hoặc không tồn tại
    if (Sequence >= SPI_SEQUENCE_MAX) {
        return E_NOT_OK;  // Sequence không hợp lệ
    }

    // Lấy cấu hình Sequence tương ứng
    const Spi_SequenceConfigType* SequenceConfig = &Spi_Sequences[Sequence];

    // Duyệt qua từng Job trong Sequence
    for (uint8_t jobIndex = 0; jobIndex < SequenceConfig->JobCount; jobIndex++) {
        Spi_JobType currentJob = SequenceConfig->Jobs[jobIndex];
        
        // Lấy cấu hình Job hiện tại
        const Spi_JobConfigType* JobConfig = &Spi_Jobs[currentJob];
        
        // Lấy kênh SPI từ JobConfig
        Spi_ChannelType channel = JobConfig->Channel;

        // Truyền dữ liệu cho Job hiện tại qua kênh tương ứng
        if (channel == SPI_CHANNEL_1) {
            // Đợi cho đến khi bộ đệm truyền của SPI1 trống
            Spi_Hw_WaitTransmitBufferEmpty_SPI1();

            // Gửi dữ liệu qua SPI1
            Spi_I2S_SendData_SPI1(&JobConfig->DataBuffer);
        } else if (channel == SPI_CHANNEL_2) {
            // Đợi cho đến khi bộ đệm truyền của SPI2 trống
            Spi_Hw_WaitTransmitBufferEmpty_SPI2();

            // Gửi dữ liệu qua SPI2
            Spi_I2S_SendData_SPI2(&JobConfig->DataBuffer);
        } else {
            return E_NOT_OK;  // Nếu kênh không hợp lệ, trả về lỗi
        }
        
        // Cập nhật trạng thái Job đã thực hiện thành công
        JobResult = SPI_JOB_OK;
    }

    // Sau khi hoàn thành tất cả các Job, cập nhật kết quả Sequence
    SeqResult = SPI_SEQ_OK;

    return E_OK;  // Truyền dữ liệu không đồng bộ thành công
}

/**********************************************************
 * @brief Đọc dữ liệu từ bộ đệm nội bộ của SPI
 * @param[in] Channel ID của kênh SPI
 * @param[out] DataBufferPtr Con trỏ tới buffer để lưu dữ liệu nhận được
 * @return E_OK nếu đọc thành công, E_NOT_OK nếu có lỗi
 **********************************************************/
Std_ReturnType Spi_ReadIB(Spi_ChannelType Channel, Spi_DataBufferType* DataBufferPtr) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Kiểm tra nếu buffer dữ liệu là NULL
    if (DataBufferPtr == NULL) {
        return E_NOT_OK;  // Buffer không hợp lệ
    }

    // Kiểm tra kênh SPI (SPI1 hoặc SPI2) và thực hiện đọc dữ liệu tương ứng
    if (Channel == SPI_CHANNEL_1) {
        // Đợi cho đến khi có dữ liệu nhận được từ SPI1
        Spi_Hw_WaitReceiveBufferFull_SPI1();

        // Đọc dữ liệu từ SPI1
        *DataBufferPtr = Spi_I2S_ReceiveData_SPI1();
    } else if (Channel == SPI_CHANNEL_2) {
        // Đợi cho đến khi có dữ liệu nhận được từ SPI2
        Spi_Hw_WaitReceiveBufferFull_SPI2();

        // Đọc dữ liệu từ SPI2
        *DataBufferPtr = Spi_I2S_ReceiveData_SPI2();
    } else {
        // Nếu không phải SPI1 hoặc SPI2, trả về lỗi
        return E_NOT_OK;
    }

    return E_OK;  // Đọc dữ liệu thành công
}

/**********************************************************
 * @brief Cấu hình bộ đệm ngoài cho kênh SPI
 * @param[in] Channel ID của kênh SPI
 * @param[in] SrcDataBufferPtr Con trỏ tới buffer nguồn (dữ liệu cần truyền)
 * @param[out] DesDataBufferPtr Con trỏ tới buffer đích (dữ liệu nhận được)
 * @param[in] Length Số lượng phần tử dữ liệu cần truyền/nhận
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 **********************************************************/
Std_ReturnType Spi_SetupEB(Spi_ChannelType Channel, const Spi_DataBufferType* SrcDataBufferPtr, Spi_DataBufferType* DesDataBufferPtr, Spi_NumberOfDataType Length) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Kiểm tra nếu buffer dữ liệu đầu vào là NULL hoặc độ dài không hợp lệ
    if ((SrcDataBufferPtr == NULL) || (DesDataBufferPtr == NULL) || (Length == 0)) {
        return E_NOT_OK;  // Buffer hoặc độ dài không hợp lệ
    }

    // Lặp qua từng phần tử dữ liệu dựa trên chiều dài buffer
    for (Spi_NumberOfDataType i = 0; i < Length; i++) {
        // Kiểm tra kênh SPI (SPI1 hoặc SPI2) và thực hiện truyền/nhận dữ liệu tương ứng
        if (Channel == SPI_CHANNEL_1) {
            // Đợi cho đến khi bộ đệm truyền của SPI1 trống
            Spi_Hw_WaitTransmitBufferEmpty_SPI1();

            // Gửi dữ liệu từ buffer nguồn qua SPI1
            Spi_I2S_SendData_SPI1(&SrcDataBufferPtr[i]);

            // Đợi dữ liệu từ SPI1
            Spi_Hw_WaitReceiveBufferFull_SPI1();

            // Đọc dữ liệu vào buffer đích từ SPI1
            DesDataBufferPtr[i] = Spi_I2S_ReceiveData_SPI1();
        } else if (Channel == SPI_CHANNEL_2) {
            // Đợi cho đến khi bộ đệm truyền của SPI2 trống
            Spi_Hw_WaitTransmitBufferEmpty_SPI2();

            // Gửi dữ liệu từ buffer nguồn qua SPI2
            Spi_I2S_SendData_SPI2(&SrcDataBufferPtr[i]);

            // Đợi dữ liệu từ SPI2
            Spi_Hw_WaitReceiveBufferFull_SPI2();

            // Đọc dữ liệu vào buffer đích từ SPI2
            DesDataBufferPtr[i] = Spi_I2S_ReceiveData_SPI2();
        } else {
            // Nếu không phải SPI1 hoặc SPI2, trả về lỗi
            return E_NOT_OK;
        }
    }

    return E_OK;  // Cấu hình bộ đệm ngoài thành công
}

/**********************************************************
 * @brief Lấy trạng thái hiện tại của SPI Handler/Driver
 * @return Trạng thái hiện tại của SPI (SPI_UNINIT, SPI_IDLE, SPI_BUSY)
 * @details Hàm này trả về trạng thái hiện tại của SPI,
 *          cho biết SPI đang ở trạng thái nào.
 **********************************************************/
Spi_StatusType Spi_GetStatus(void) {
    // Nếu SPI chưa được khởi tạo, trả về SPI_UNINIT
    if (SpiStatus == SPI_UNINIT) {
        return SPI_UNINIT;
    }

    // Kiểm tra trạng thái của SPI1
    if (Spi_Hw_CheckStatus_SPI1() == SPI_BUSY) {
        return SPI_BUSY;  // SPI1 đang bận
    }

    // Kiểm tra trạng thái của SPI2
    if (Spi_Hw_CheckStatus_SPI2() == SPI_BUSY) {
        return SPI_BUSY;  // SPI2 đang bận
    }

    // Nếu tất cả SPI đều không bận, trả về SPI_IDLE
    return SPI_IDLE;
}

/**********************************************************
 * @brief Lấy kết quả của một Job SPI
 * @param[in] Job ID của Job SPI
 * @return Kết quả của Job (SPI_JOB_OK, SPI_JOB_PENDING, SPI_JOB_FAILED)
 * @details Hàm này trả về kết quả của một Job SPI đã được truyền.
 **********************************************************/
Spi_JobResultType Spi_GetJobResult(Spi_JobType Job) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return SPI_JOB_FAILED;  // SPI chưa được khởi tạo
    }

    // Dựa trên Job đang được thực hiện, kiểm tra kênh SPI tương ứng
    if (Job == SPI_JOB_READ_TEMP_SENSOR) {
        // Kiểm tra trạng thái của Job trên SPI1
        return Spi_Hw_CheckJobStatus_SPI1();
    } else if (Job == SPI_JOB_WRITE_EEPROM || Job == SPI_JOB_READ_EEPROM) {
        // Kiểm tra trạng thái của Job trên SPI2
        return Spi_Hw_CheckJobStatus_SPI2();
    } else {
        // Nếu Job không hợp lệ, trả về SPI_JOB_FAILED
        return SPI_JOB_FAILED;
    }
}

/**********************************************************
 * @brief Lấy kết quả của một Sequence SPI
 * @param[in] Sequence ID của Sequence SPI
 * @return Kết quả của Sequence (SPI_SEQ_OK, SPI_SEQ_PENDING, SPI_SEQ_FAILED)
 * @details Hàm này trả về kết quả của một Sequence SPI đã được truyền.
 **********************************************************/
Spi_SeqResultType Spi_GetSequenceResult(Spi_SequenceType Sequence) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return SPI_SEQ_FAILED;  // SPI chưa được khởi tạo
    }

    // Dựa trên Sequence đang được thực hiện, kiểm tra kênh SPI tương ứng
    if (Sequence == SPI_SEQUENCE_0) {
        // Kiểm tra trạng thái của Sequence trên SPI1
        return Spi_Hw_CheckSequenceStatus_SPI1();
    } else if (Sequence == SPI_SEQUENCE_1 || Sequence == SPI_SEQUENCE_2) {
        // Kiểm tra trạng thái của Sequence trên SPI2
        return Spi_Hw_CheckSequenceStatus_SPI2();
    } else {
        // Nếu Sequence không hợp lệ, trả về SPI_SEQ_FAILED
        return SPI_SEQ_FAILED;
    }
}

/**********************************************************
 * @brief Lấy thông tin phiên bản của SPI Handler/Driver
 * @param[out] VersionInfo Con trỏ tới cấu trúc Std_VersionInfoType chứa thông tin phiên bản
 * @details Hàm này trả về thông tin phiên bản của SPI Handler/Driver,
 *          bao gồm vendorID, moduleID, và các phiên bản phần mềm.
 **********************************************************/
void Spi_GetVersionInfo(Std_VersionInfoType* VersionInfo) {
    // Kiểm tra nếu con trỏ VersionInfo là NULL
    if (VersionInfo == NULL) {
        return;  // Nếu con trỏ không hợp lệ, không làm gì cả
    }

    // Gán thông tin về vendor ID và module ID
    VersionInfo->vendorID = 1;          // Giả định Vendor ID là 1
    VersionInfo->moduleID = 123;        // Giả định Module ID cho SPI là 123

    // Gán các thông tin về phiên bản phần mềm
    VersionInfo->sw_major_version = 1;  // Phiên bản chính (major) là 1
    VersionInfo->sw_minor_version = 0;  // Phiên bản phụ (minor) là 0
    VersionInfo->sw_patch_version = 0;  // Phiên bản sửa lỗi (patch) là 0
}

/**********************************************************
 * @brief Truyền dữ liệu đồng bộ qua SPI
 * @param[in] Sequence ID của Sequence cần truyền
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Truyền dữ liệu đồng bộ qua Sequence SPI, đợi quá trình truyền hoàn thành.
 **********************************************************/
Std_ReturnType Spi_SyncTransmit(Spi_SequenceType Sequence) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Gọi hàm truyền dữ liệu không đồng bộ trước để bắt đầu quá trình truyền
    Std_ReturnType result = Spi_AsyncTransmit(Sequence);
    if (result != E_OK) {
        return E_NOT_OK;  // Nếu không thể truyền không đồng bộ, trả về lỗi
    }

    // Đợi cho đến khi tất cả các Job trong Sequence được hoàn tất
    Spi_SeqResultType seqResult;
    do {
        seqResult = Spi_GetSequenceResult(Sequence);
    } while (seqResult == SPI_SEQ_PENDING);  // Tiếp tục đợi nếu Sequence vẫn đang thực hiện

    // Kiểm tra kết quả cuối cùng của Sequence
    if (seqResult == SPI_SEQ_OK) {
        return E_OK;  // Truyền đồng bộ thành công
    } else {
        return E_NOT_OK;  // Truyền thất bại
    }
}

/**********************************************************
 * @brief Lấy trạng thái phần cứng của đơn vị SPI
 * @return Trạng thái hiện tại của phần cứng SPI (SPI_BUSY, SPI_IDLE)
 * @details Hàm này trả về trạng thái của phần cứng SPI, kiểm tra
 *          cả SPI1 và SPI2 để xác định xem có đơn vị nào đang bận hay không.
 **********************************************************/
Spi_StatusType Spi_GetHWUnitStatus(void) {
    // Kiểm tra trạng thái phần cứng của SPI1
    if (Spi_Hw_CheckHWStatus_SPI1() == SPI_BUSY) {
        return SPI_BUSY;  // Nếu SPI1 đang bận, trả về SPI_BUSY
    }

    // Kiểm tra trạng thái phần cứng của SPI2
    if (Spi_Hw_CheckHWStatus_SPI2() == SPI_BUSY) {
        return SPI_BUSY;  // Nếu SPI2 đang bận, trả về SPI_BUSY
    }

    // Nếu cả SPI1 và SPI2 đều không bận, trả về SPI_IDLE
    return SPI_IDLE;
}

/**********************************************************
 * @brief Hủy một Sequence đang truyền
 * @param[in] Sequence ID của Sequence cần hủy
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Hủy quá trình truyền của một Sequence hiện tại và
 *          dừng quá trình truyền dữ liệu.
 **********************************************************/
Std_ReturnType Spi_Cancel(Spi_SequenceType Sequence) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Dựa trên Sequence, hủy quá trình truyền trên kênh SPI tương ứng
    if (Sequence == SPI_SEQUENCE_0) {
        // Hủy quá trình truyền trên SPI1
        Spi_Hw_Cancel_SPI1();
    } else if (Sequence == SPI_SEQUENCE_1 || Sequence == SPI_SEQUENCE_2) {
        // Hủy quá trình truyền trên SPI2
        Spi_Hw_Cancel_SPI2();
    } else {
        // Sequence không hợp lệ
        return E_NOT_OK;
    }

    // Đặt trạng thái của Sequence thành CANCELED
    SeqResult = SPI_SEQ_CANCELED;

    return E_OK;  // Hủy thành công
}

/**********************************************************
 * @brief Cài đặt chế độ truyền không đồng bộ cho SPI
 * @param[in] Mode Chế độ truyền không đồng bộ (SPI_POLLING_MODE hoặc SPI_INTERRUPT_MODE)
 * @return E_OK nếu thành công, E_NOT_OK nếu có lỗi
 * @details Cài đặt chế độ không đồng bộ cho quá trình truyền dữ liệu của SPI,
 *          sử dụng chế độ thăm dò (polling) hoặc ngắt (interrupt).
 **********************************************************/
Std_ReturnType Spi_SetAsyncMode(Spi_AsyncModeType Mode) {
    // Kiểm tra nếu SPI chưa được khởi tạo
    if (SpiStatus == SPI_UNINIT) {
        return E_NOT_OK;  // SPI chưa được khởi tạo
    }

    // Kiểm tra chế độ cần chuyển đổi
    if (Mode == SPI_POLLING_MODE) {
        // Chuyển SPI sang chế độ Polling
        Spi_Hw_SetPollingMode();
        SpiStatus = SPI_IDLE;  // Đặt trạng thái SPI về IDLE trong chế độ Polling
    } else if (Mode == SPI_INTERRUPT_MODE) {
        // Chuyển SPI sang chế độ Interrupt
        Spi_Hw_SetInterruptMode();
        SpiStatus = SPI_IDLE;  // Đặt trạng thái SPI về IDLE trong chế độ Interrupt
    } else {
        // Nếu mode không hợp lệ, trả về lỗi
        return E_NOT_OK;
    }

    // Chế độ được chuyển đổi thành công
    return E_OK;
}

/**********************************************************
 * @brief Hàm chính xử lý các tác vụ của SPI
 * @details Hàm này được gọi định kỳ để xử lý các Job và Sequence của SPI.
 *          Nó quản lý trạng thái SPI, kiểm tra quá trình truyền/nhận dữ liệu
 *          và đảm bảo mọi tác vụ không đồng bộ được xử lý.
 **********************************************************/
void Spi_MainFunction_Handling(void) {
    // Kiểm tra trạng thái của SPI
    if (SpiStatus == SPI_UNINIT) {
        return;  // Nếu SPI chưa được khởi tạo, không làm gì cả
    }

    // Kiểm tra trạng thái hiện tại của SPI1 và SPI2
    Spi_StatusType spi1Status = Spi_Hw_CheckHWStatus_SPI1();
    Spi_StatusType spi2Status = Spi_Hw_CheckHWStatus_SPI2();

    // Nếu SPI1 đang bận, xử lý các tác vụ liên quan đến SPI1
    if (spi1Status == SPI_BUSY) {
        // Kiểm tra nếu có Job đang được thực thi trên SPI1
        if (JobResult == SPI_JOB_PENDING) {
            // Giả lập quá trình hoàn thành Job trên SPI1
            JobResult = SPI_JOB_OK;  // Giả định Job đã hoàn thành
        }
    }

    // Nếu SPI2 đang bận, xử lý các tác vụ liên quan đến SPI2
    if (spi2Status == SPI_BUSY) {
        // Kiểm tra nếu có Job đang được thực thi trên SPI2
        if (JobResult == SPI_JOB_PENDING) {
            // Giả lập quá trình hoàn thành Job trên SPI2
            JobResult = SPI_JOB_OK;  // Giả định Job đã hoàn thành
        }
    }

    // Cập nhật trạng thái của các Sequence SPI
    if (SeqResult == SPI_SEQ_PENDING) {
        // Kiểm tra nếu Sequence đang thực hiện và xử lý
        if (JobResult == SPI_JOB_OK) {
            SeqResult = SPI_SEQ_OK;  // Cập nhật trạng thái Sequence khi tất cả các Job đã hoàn thành
        } else {
            SeqResult = SPI_SEQ_FAILED;  // Nếu có lỗi xảy ra, đánh dấu Sequence là thất bại
        }
    }
}

