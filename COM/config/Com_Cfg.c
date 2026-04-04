/**********************************************************
 * @file    Com_Cfg.c
 * @brief   Bảng cấu hình COM – 3 CAN I-PDU + 2 LIN I-PDU
 **********************************************************/
#include "Com_Cfg.h"

const Com_IPduCfgType Com_IPduCfg[COM_NUM_IPDUS] = {
    /* CAN I-PDUs */
    { .PduId = ComConf_ComIPdu_EngineCmd,  .Length = 5u, .direction = COM_PDU_DIR_TX },
    { .PduId = ComConf_ComIPdu_BrakeCmd,   .Length = 3u, .direction = COM_PDU_DIR_TX },
    { .PduId = ComConf_ComIPdu_BodyCmd,    .Length = 4u, .direction = COM_PDU_DIR_TX },
    /* LIN I-PDUs */
    { .PduId = ComConf_ComIPdu_LightCtrl,  .Length = 4u, .direction = COM_PDU_DIR_TX },
    { .PduId = ComConf_ComIPdu_HVACCtrl,   .Length = 3u, .direction = COM_PDU_DIR_TX }
};

const Com_SignalCfgType Com_SignalCfg[COM_NUM_SIGNALS] = {   //0xc8 (17 ô nhớ)
    /* ===== EngineCmd (CAN 0x180) ===== */
    [ComSig_Engine_Throttle]    = { .PduId = ComConf_ComIPdu_EngineCmd, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },
    [ComSig_Engine_Start]       = { .PduId = ComConf_ComIPdu_EngineCmd, .byteIndex = 1u, .bitOffset = 0u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },
    [ComSig_Engine_TorqueLimit] = { .PduId = ComConf_ComIPdu_EngineCmd, .byteIndex = 2u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },
    [ComSig_Engine_Alive]       = { .PduId = ComConf_ComIPdu_EngineCmd, .byteIndex = 3u, .bitOffset = 0u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },
    [ComSig_Engine_CRC]         = { .PduId = ComConf_ComIPdu_EngineCmd, .byteIndex = 3u, .bitOffset = 4u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },

    /* ===== BrakeCmd (CAN 0x280) ===== */
    [ComSig_Brake_BrakeReq] = { .PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },
    [ComSig_Brake_RegenReq] = { .PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 1u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },
    [ComSig_Brake_Alive]    = { .PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 2u, .bitOffset = 0u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },
    [ComSig_Brake_CRC]      = { .PduId = ComConf_ComIPdu_BrakeCmd, .byteIndex = 2u, .bitOffset = 4u, .bitLength = 4u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },

    /* ===== BodyCmd (CAN 0x380) ===== */
    [ComSig_Body_Headlamp] = { .PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },
    [ComSig_Body_TurnL]    = { .PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 1u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },
    [ComSig_Body_TurnR]    = { .PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 2u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },
    [ComSig_Body_DoorLock] = { .PduId = ComConf_ComIPdu_BodyCmd, .byteIndex = 0u, .bitOffset = 3u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },

    /* ===== LightCtrl (LIN 0x10) ===== */
    [ComSig_Light_Headlamp]   = { .PduId = ComConf_ComIPdu_LightCtrl, .byteIndex = 0u, .bitOffset = 0u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },
    [ComSig_Light_DRL]        = { .PduId = ComConf_ComIPdu_LightCtrl, .byteIndex = 0u, .bitOffset = 1u, .bitLength = 1u, .type = COM_SIGTYPE_BOOLEAN, .direction = COM_PDU_DIR_TX },
    [ComSig_Light_Brightness] = { .PduId = ComConf_ComIPdu_LightCtrl, .byteIndex = 1u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX },

    /* ===== HVACCtrl (LIN 0x11) ===== */
    [ComSig_HVAC_FanSpeed]    = { .PduId = ComConf_ComIPdu_HVACCtrl,  .byteIndex = 0u, .bitOffset = 0u, .bitLength = 8u, .type = COM_SIGTYPE_UINT8,   .direction = COM_PDU_DIR_TX }
};
