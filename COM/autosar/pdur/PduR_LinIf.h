/**********************************************************
 * @file    PduR_LinIf.h
 * @brief   Giao diện PduR cho lower layer LinIf
 * @details Callback mà LinIf sẽ gọi lên PduR:
 *            - PduR_LinIfRxIndication()
 *            - PduR_LinIfTxConfirmation()
 *          Và API PduR gọi xuống LinIf:
 *            - LinIf_Transmit() (extern, LinIf cung cấp)
 **********************************************************/
#ifndef PDUR_LINIF_H
#define PDUR_LINIF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Std_Types.h"
#include "ComStack_Types.h"

/* LinIf → PduR: I-PDU đã nhận xong từ LIN bus */
void PduR_LinIfRxIndication(PduIdType LinIfRxPduId, const PduInfoType* PduInfoPtr);

/* LinIf → PduR: I-PDU TX đã xác nhận xong qua LIN */
void PduR_LinIfTxConfirmation(PduIdType LinIfTxPduId);

#ifdef __cplusplus
}
#endif

#endif /* PDUR_LINIF_H */
