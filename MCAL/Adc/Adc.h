/**********************************************************
 * @file    Adc.h
 * @brief   Analog-to-Digital Converter (ADC) Driver Header
 * @details Khai báo kiểu dữ liệu và API ADC Driver theo chuẩn AUTOSAR,
 *          target STM32F103 sử dụng thư viện SPL.
 *          Lưu ý: ADC Driver chỉ quản lý ngoại vi ADC/DMA/IRQ.
 *          Cấu hình chân GPIO analog thuộc trách nhiệm Port Driver.
 * @version 2.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#ifndef ADC_H
#define ADC_H

#include "Std_Types.h"
#include "Adc_Types.h"

/**********************************************************
 * API chuẩn SWS ADC
 **********************************************************/
/**********************************************************
 * @brief   Khởi tạo ADC Driver với bộ cấu hình được chọn
 * @details Hàm này chỉ cấu hình phần ADC peripheral (clock, init, calibration,
 *          IRQ/DMA baseline). Cấu hình chân GPIO analog phải do Port Driver xử lý.
 * @param[in] ConfigPtr Con trỏ tới cấu hình ADC tổng (post-build/link-time)
 **********************************************************/
void Adc_Init(const Adc_ConfigType *ConfigPtr);

/**********************************************************
 * @brief   Gán buffer kết quả cho một ADC Group
 * @details Buffer này là vùng nhớ đích để driver lưu kết quả chuyển đổi
 *          (single hoặc streaming). API này cần gọi trước khi start conversion.
 * @param[in] Group         ID group logic cần gán buffer
 * @param[in] DataBufferPtr Con trỏ tới vùng dữ liệu kết quả
 * @return  E_OK nếu thành công, E_NOT_OK nếu tham số/trạng thái không hợp lệ
 **********************************************************/
Std_ReturnType Adc_SetupResultBuffer(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

/**********************************************************
 * @brief   Đưa ADC Driver về trạng thái de-initialize
 * @details Tắt conversion/trigger/IRQ/DMA và reset runtime state của driver.
 *          API này chỉ có hiệu lực khi `ADC_DEINIT_API == STD_ON`.
 **********************************************************/
void Adc_DeInit(void);

/**********************************************************
 * @brief   Bắt đầu chuyển đổi cho một group
 * @details Với group software-trigger: driver sẽ phát lệnh start conversion.
 *          Với group hardware-trigger: cần bật trigger trước bằng API tương ứng.
 * @param[in] Group ID group logic cần bắt đầu chuyển đổi
 **********************************************************/
void Adc_StartGroupConversion(Adc_GroupType Group);

/**********************************************************
 * @brief   Dừng chuyển đổi của một group
 * @details API này dừng conversion và đưa trạng thái group về IDLE.
 * @param[in] Group ID group logic cần dừng chuyển đổi
 **********************************************************/
void Adc_StopGroupConversion(Adc_GroupType Group);

/**********************************************************
 * @brief   Đọc kết quả chuyển đổi mới nhất của group
 * @details Kết quả sẽ được copy ra vùng nhớ `DataBufferPtr` theo cấu hình group.
 *          API này chỉ có hiệu lực khi `ADC_READ_GROUP_API == STD_ON`.
 * @param[in] Group         ID group logic cần đọc dữ liệu
 * @param[out] DataBufferPtr Con trỏ vùng nhớ nhận dữ liệu
 * @return  E_OK nếu đọc thành công, E_NOT_OK nếu không có dữ liệu hợp lệ hoặc sai precondition
 **********************************************************/
Std_ReturnType Adc_ReadGroup(Adc_GroupType Group, Adc_ValueGroupType *DataBufferPtr);

/**********************************************************
 * @brief   Bật hardware trigger cho group
 * @details Chỉ áp dụng cho group cấu hình trigger nguồn phần cứng.
 *          API này chỉ có hiệu lực khi `ADC_HW_TRIGGER_API == STD_ON`.
 * @param[in] Group ID group logic cần bật hardware trigger
 **********************************************************/
void Adc_EnableHardwareTrigger(Adc_GroupType Group);

/**********************************************************
 * @brief   Tắt hardware trigger cho group
 * @details Chỉ áp dụng cho group cấu hình trigger nguồn phần cứng.
 *          API này chỉ có hiệu lực khi `ADC_HW_TRIGGER_API == STD_ON`.
 * @param[in] Group ID group logic cần tắt hardware trigger
 **********************************************************/
void Adc_DisableHardwareTrigger(Adc_GroupType Group);

/**********************************************************
 * @brief   Bật notification callback cho group
 * @details Khi EOC interrupt của ADC unit xảy ra, callback group tương ứng
 *          sẽ được gọi nếu đã bật notification.
 * @param[in] Group ID group logic cần bật notification
 **********************************************************/
void Adc_EnableGroupNotification(Adc_GroupType Group);

/**********************************************************
 * @brief   Tắt notification callback cho group
 * @details Sau khi tắt, callback của group sẽ không còn được dispatch từ ISR.
 *          Nếu không còn group nào bật notification trên cùng ADC unit,
 *          driver có thể disable EOC interrupt của unit đó để giảm tải ngắt.
 * @param[in] Group ID group logic cần tắt notification
 **********************************************************/
void Adc_DisableGroupNotification(Adc_GroupType Group);

/**********************************************************
 * @brief   Lấy trạng thái hiện tại của group
 * @details Trạng thái phản ánh runtime state do ADC driver quản lý nội bộ.
 *          Nếu module chưa khởi tạo hoặc group không hợp lệ, giá trị trả về an toàn là `ADC_IDLE`.
 * @param[in] Group ID group logic cần đọc trạng thái
 * @return  Giá trị trong `Adc_StatusType` (IDLE/BUSY/COMPLETED/STREAM_COMPLETED)
 **********************************************************/
Adc_StatusType Adc_GetGroupStatus(Adc_GroupType Group);

/**********************************************************
 * @brief   Lấy con trỏ mẫu cuối và số lượng mẫu hợp lệ của group
 * @details Dùng cho access mode streaming để upper layer lấy vùng dữ liệu gần nhất.
 * @param[in] Group ID group logic cần truy vấn
 * @param[out] PtrToSamplePtr Con trỏ nhận địa chỉ mẫu
 * @return  Số lượng mẫu hợp lệ hiện có trong buffer
 **********************************************************/
Adc_StreamNumSampleType Adc_GetStreamLastPointer(Adc_GroupType Group, Adc_ValueGroupType **PtrToSamplePtr);

/**********************************************************
 * @brief   Lấy thông tin version của ADC Driver
 * @details API điền vendor ID, module ID và software version theo macro cấu hình.
 *          API này chỉ có hiệu lực khi `ADC_VERSION_INFO_API == STD_ON`.
 * @param[out] VersionInfo Con trỏ cấu trúc nhận thông tin version
 **********************************************************/
void Adc_GetVersionInfo(Std_VersionInfoType *VersionInfo);

/**********************************************************
 * @brief   Áp trạng thái nguồn đã chuẩn bị cho ADC Driver
 * @details API này thực hiện bước apply sau khi đã gọi `Adc_PreparePowerState`.
 *          Nếu chưa prepare hoặc target không hợp lệ, hàm trả lỗi theo quy ước AUTOSAR.
 * @param[out] Result Con trỏ nhận kết quả yêu cầu power state
 * @return  E_OK nếu chuyển thành công, E_NOT_OK nếu lỗi precondition/trạng thái
 **********************************************************/
Std_ReturnType Adc_SetPowerState(Adc_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Lấy trạng thái nguồn hiện tại của ADC Driver
 * @details Trạng thái này phản ánh mức nguồn mà driver đang áp dụng thực tế cho ADC unit.
 * @param[out] CurrentPowerState Con trỏ nhận trạng thái nguồn hiện tại
 * @param[out] Result            Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu đọc thành công, E_NOT_OK nếu lỗi precondition
 **********************************************************/
Std_ReturnType Adc_GetCurrentPowerState(Adc_PowerStateType *CurrentPowerState,
                                        Adc_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Lấy trạng thái nguồn mục tiêu đang được chuẩn bị
 * @details Trạng thái này được set bởi `Adc_PreparePowerState` và dùng cho bước apply.
 * @param[out] TargetPowerState Con trỏ nhận trạng thái nguồn mục tiêu
 * @param[out] Result           Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu đọc thành công, E_NOT_OK nếu lỗi precondition
 **********************************************************/
Std_ReturnType Adc_GetTargetPowerState(Adc_PowerStateType *TargetPowerState,
                                       Adc_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Chuẩn bị trạng thái nguồn mục tiêu cho ADC Driver
 * @details API chỉ ghi nhận mục tiêu chuyển trạng thái; chưa áp phần cứng ngay.
 *          Việc áp thực tế được thực hiện khi gọi `Adc_SetPowerState`.
 * @param[in] PowerState Trạng thái nguồn mục tiêu cần chuẩn bị
 * @param[out] Result    Con trỏ nhận kết quả dịch vụ
 * @return  E_OK nếu chuẩn bị thành công, E_NOT_OK nếu không hỗ trợ/không hợp lệ
 **********************************************************/
Std_ReturnType Adc_PreparePowerState(Adc_PowerStateType PowerState,
                                     Adc_PowerStateRequestResultType *Result);

/**********************************************************
 * @brief   Hàm nền quản lý chuyển trạng thái nguồn bất đồng bộ
 * @details Chỉ hoạt động khi `ADC_POWER_STATE_ASYNCH_TRANSITION_MODE == STD_ON`.
 **********************************************************/
void Adc_Main_PowerTransitionManager(void);

/**********************************************************
 * API nội bộ cho adapter IRQ (MCAL integration layer)
 * - Được gọi từ vector IRQ trong file cấu hình Adc_Cfg.c
 **********************************************************/
/**********************************************************
 * @brief   Handler logic cho ngắt ADCx
 * @details Hàm này không phải vector IRQ trực tiếp; vector sẽ gọi vào đây.
 * @param[in] AdcInstance Con trỏ ADC instance phát sinh ngắt
 **********************************************************/
void Adc_IsrHandler(ADC_TypeDef *AdcInstance);

/**********************************************************
 * @brief   Handler logic cho ngắt DMA channel của ADC
 * @details Hàm này xử lý HT/TC flag và dispatch callback DMA theo group.
 * @param[in] DmaChannel Con trỏ DMA channel phát sinh ngắt
 **********************************************************/
void Adc_DmaIsrHandler(DMA_Channel_TypeDef *DmaChannel);

#endif /* ADC_H */
