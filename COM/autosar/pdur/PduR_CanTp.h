#ifndef PDUR_CANTP_H
#define PDUR_CANTP_H

#include "Std_Types.h"
#include "ComStack_Types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Các API callback mà CanTp sẽ gọi lên PduR */

void PduR_CanTpRxIndication(PduIdType RxPduId, NotifResultType Result);
void PduR_CanTpTxConfirmation(PduIdType TxPduId, NotifResultType Result);

#ifdef __cplusplus
}
#endif
#endif /* PDUR_CANTP_H */
