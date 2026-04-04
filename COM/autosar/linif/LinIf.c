/**********************************************************
 * @file    LinIf.c
 * @brief   AUTOSAR LIN Interface (LinIf) – PduR ↔ LIN Driver
 *
 * @details Tầng ECU Abstraction kết nối PduR (phía trên) với
 *          LIN Driver (phía dưới). Chịu trách nhiệm:
 *
 *          1. Nhận I-PDU từ PduR (LinIf_Transmit)
 *          2. Buffer dữ liệu vào pending queue
 *          3. Gửi theo lịch trình trong LinIf_MainFunction
 *          4. Chuyển đổi PduInfoType → Lin_PduType
 *          5. Gọi Lin_SendFrame() xuống LIN Driver
 *          6. Báo TX confirm ngược lên PduR → COM
 *
 *          Luồng TX:
 *          ┌────────┐  LinIf_Transmit(id, &info)  ┌────────┐
 *          │  PduR  │ ──────────────────────────►  │ LinIf  │
 *          └────────┘                              └────┬───┘
 *                       LinIf_MainFunction()            │
 *                       (gọi định kỳ)                   │
 *                                                       │ Lin_SendFrame(ch, &pdu)
 *                                                  ┌────▼───────┐
 *                                                  │ LIN Driver │
 *                                                  │  (USART2)  │
 *                                                  └────────────┘
 *
 *          Bảng mapping LinIf PDU → LIN Frame ID:
 *          ┌──────────────────────┬───────────────┬────┐
 *          │ LinIfTxPduId         │ LIN Frame ID  │DLC │
 *          ├──────────────────────┼───────────────┼────┤
 *          │ 0 (LightCtrl)       │ 0x10          │ 4  │
 *          │ 1 (HVACCtrl)        │ 0x11          │ 3  │
 *          └──────────────────────┴───────────────┴────┘
 *
 *          Lưu ý: LIN là half-duplex, mỗi lần MainFunction
 *          chỉ gửi 1 frame để tránh xung đột trên bus.
 *
 * @version 2.0
 * @date    2025-09-19
 * @author  HALA Academy
 **********************************************************/

#include "LinIf.h"
#include "Lin.h"               /* Lin_SendFrame() – API LIN Driver    */
#include "PduR_LinIf.h"        /* PduR_LinIfTxConfirmation() callback  */
#include <stddef.h>
#include <string.h>

/* ===========================================================
 * Hằng số cấu hình
 * ===========================================================*/
#define LINIF_NUM_TX_PDUS    2u   /**< Số lượng TX PDU: LightCtrl + HVACCtrl */
#define LINIF_CHANNEL        0u   /**< Kênh LIN duy nhất trong demo          */

/* ===========================================================
 * LinIf_TxPduCfgType – Cấu hình TX PDU cho LinIf
 * -----------------------------------------------------------
 * Mỗi entry map: LinIfTxPduId → LIN Frame ID + DLC tối đa
 * ===========================================================*/
typedef struct {
    PduIdType  LinIfTxPduId;    /**< PDU ID phía LinIf (từ PduR route) */
    uint8      LinFrameId;      /**< LIN frame ID trên bus (6-bit)     */
    uint8      DlcMax;          /**< DLC tối đa cho phép               */
} LinIf_TxPduCfgType;

/* Bảng cấu hình tĩnh (const, lưu trong Flash) */
static const LinIf_TxPduCfgType LinIf_TxPduCfg[LINIF_NUM_TX_PDUS] = {
    { .LinIfTxPduId = 0u, .LinFrameId = 0x10u, .DlcMax = 4u },  /* LightCtrl */
    { .LinIfTxPduId = 1u, .LinFrameId = 0x11u, .DlcMax = 3u }   /* HVACCtrl  */
};

/* ===========================================================
 * Biến trạng thái nội bộ
 * ===========================================================*/
static boolean s_linIfInited = FALSE;  /**< Cờ đã khởi tạo */

/* ===========================================================
 * LinIf_TxBufType – Buffer chờ gửi (TX pending)
 * -----------------------------------------------------------
 * Vì LIN là half-duplex và có schedule phức tạp, LinIf
 * không gửi ngay khi nhận được Transmit request. Thay vào đó,
 * dữ liệu được lưu vào buffer pending, rồi MainFunction
 * sẽ lần lượt gửi từng frame.
 * ===========================================================*/
typedef struct {
    uint8     Data[8];      /**< Dữ liệu payload (copy từ PduInfo)  */
    uint8     Dlc;          /**< Độ dài dữ liệu thực tế             */
    uint8     LinFrameId;   /**< LIN frame ID tương ứng              */
    PduIdType PduId;        /**< PDU ID (để callback confirm)        */
    boolean   Pending;      /**< TRUE = có data chờ gửi             */
} LinIf_TxBufType;

static LinIf_TxBufType s_txBuf[LINIF_NUM_TX_PDUS];

/* ===========================================================
 * prv_find_txpdu – Tìm index TX PDU trong bảng cấu hình
 * ===========================================================*/
static inline int16_t prv_find_txpdu(PduIdType id)
{
    for (uint8 i = 0; i < LINIF_NUM_TX_PDUS; i++) {
        if (LinIf_TxPduCfg[i].LinIfTxPduId == id) {
            return (int16_t)i;
        }
    }
    return (int16_t)-1;
}

/* ===========================================================
 * LinIf_Init – Khởi tạo LinIf module
 * -----------------------------------------------------------
 * 1. Gọi Lin_Init() để khởi tạo LIN Driver (USART2 + GPIO)
 * 2. Clear tất cả TX buffer pending
 * ===========================================================*/
void LinIf_Init(void)
{
    /* Khởi tạo LIN Driver MCAL bên dưới */
    Lin_Init();

    /* Clear tất cả buffer pending */
    (void)memset(s_txBuf, 0, sizeof(s_txBuf));

    s_linIfInited = TRUE;
}

/* ===========================================================
 * LinIf_Transmit – PduR gọi: xếp I-PDU vào queue gửi LIN
 * -----------------------------------------------------------
 * Không gửi ngay! Chỉ copy dữ liệu vào buffer pending.
 * MainFunction sẽ thật sự gửi qua LIN Driver.
 *
 * Quy trình:
 *   1. Kiểm tra param hợp lệ
 *   2. Tra bảng config → tìm LinFrameId + DlcMax
 *   3. Copy payload vào s_txBuf[idx]
 *   4. Set Pending = TRUE
 *   5. Return E_OK (đã xếp lịch gửi)
 * ===========================================================*/
Std_ReturnType LinIf_Transmit(PduIdType TxPduId, const PduInfoType* PduInfoPtr)
{
    /* Kiểm tra điều kiện */
    if (!s_linIfInited || (PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL)) {
        return E_NOT_OK;
    }

    /* Tra bảng config */
    int16_t idx = prv_find_txpdu(TxPduId);
    if (idx < 0) {
        return E_NOT_OK;
    }

    const LinIf_TxPduCfgType* cfg = &LinIf_TxPduCfg[(uint8)idx];

    /* Kiểm tra DLC không vượt quá config */
    if (PduInfoPtr->SduLength > cfg->DlcMax) {
        return E_NOT_OK;
    }

    /* Copy dữ liệu vào buffer pending */
    uint8 len = (PduInfoPtr->SduLength <= 8u) ? (uint8)PduInfoPtr->SduLength : 8u;
    s_txBuf[(uint8)idx].PduId      = TxPduId;
    s_txBuf[(uint8)idx].Dlc        = len;
    s_txBuf[(uint8)idx].LinFrameId = cfg->LinFrameId;
    s_txBuf[(uint8)idx].Pending    = TRUE;
    (void)memcpy(s_txBuf[(uint8)idx].Data, PduInfoPtr->SduDataPtr, len);

    return E_OK;
}

/* ===========================================================
 * LinIf_MainFunction – Gửi pending LIN frames
 * -----------------------------------------------------------
 * Gọi định kỳ trong vòng lặp chính (khoảng 5-10ms).
 *
 * Quy trình:
 *   1. Duyệt tất cả buffer pending
 *   2. Tìm buffer có Pending = TRUE
 *   3. Tạo Lin_PduType:
 *      - Pid = LinFrameId (LIN Driver sẽ tính parity)
 *      - Cs  = Enhanced (LIN 2.x)
 *      - Drc = TX (master gửi response)
 *   4. Gọi Lin_SendFrame()
 *   5. Nếu thành công → clear Pending, gọi TxConfirmation lên PduR
 *   6. BREAK sau khi gửi 1 frame (LIN half-duplex)
 *
 * Lưu ý: Chỉ gửi 1 frame mỗi lần MainFunction vì LIN là
 * bus half-duplex single-master, gửi nhiều frame liên tiếp
 * có thể gây lỗi timing.
 * ===========================================================*/
void LinIf_MainFunction(void)
{
    if (!s_linIfInited) return;

    for (uint8 i = 0; i < LINIF_NUM_TX_PDUS; i++) {
        if (s_txBuf[i].Pending) {
            /* Tạo Lin_PduType cho LIN Driver */
            Lin_PduType linPdu;
            linPdu.Pid    = s_txBuf[i].LinFrameId; /* LIN frame ID */
            linPdu.Cs     = LIN_ENHANCED_CS;        /* Enhanced checksum (LIN 2.x) */
            linPdu.Drc    = LIN_FRAMERESPONSE_TX;   /* Master gửi response */
            linPdu.Dl     = s_txBuf[i].Dlc;         /* Độ dài data */
            linPdu.SduPtr = s_txBuf[i].Data;        /* Con trỏ data */

            /* Gọi LIN Driver để gửi frame */
            Std_ReturnType ret = Lin_SendFrame(LINIF_CHANNEL, &linPdu);

            if (ret == E_OK) {
                /* Gửi thành công → clear pending */
                s_txBuf[i].Pending = FALSE;

                /* Báo TX confirm lên PduR → COM */
                PduR_LinIfTxConfirmation(s_txBuf[i].PduId);
            }

            /* QUAN TRỌNG: chỉ gửi 1 frame mỗi lần MainFunction
             * LIN là half-duplex, cần chờ frame hoàn tất trước khi gửi tiếp */
            break;
        }
    }
}

/* ===========================================================
 * LinIf_GotoSleep – Đưa bus LIN vào sleep mode
 * -----------------------------------------------------------
 * Chuyển tiếp xuống LIN Driver: Lin_GoToSleep()
 * ===========================================================*/
Std_ReturnType LinIf_GotoSleep(uint8 Channel)
{
    if (!s_linIfInited) return E_NOT_OK;
    return Lin_GoToSleep(Channel);
}

/* ===========================================================
 * LinIf_Wakeup – Đánh thức bus LIN
 * -----------------------------------------------------------
 * Chuyển tiếp xuống LIN Driver: Lin_Wakeup()
 * ===========================================================*/
Std_ReturnType LinIf_Wakeup(uint8 Channel)
{
    if (!s_linIfInited) return E_NOT_OK;
    return Lin_Wakeup(Channel);
}
