
#ifndef _BT_PROFILE_CFG_H_
#define _BT_PROFILE_CFG_H_

#include "app_config.h"
#include "btcontroller_modules.h"


#if (DUEROS_DMA_EN || TRANS_DATA_EN || RCSP_BTMATE_EN || RCSP_ADV_EN || GMA_EN || SMART_BOX_EN)
#define    BT_FOR_APP_EN             1
#else
#define    BT_FOR_APP_EN             0
#endif


///---sdp service record profile- 用户选择支持协议--///
#if (BT_FOR_APP_EN)
#undef USER_SUPPORT_PROFILE_SPP    
#define USER_SUPPORT_PROFILE_SPP    1
#endif

//ble demo的例子
#define DEF_BLE_DEMO_NULL                 0 //ble 没有使能
#define DEF_BLE_DEMO_ADV                  1 //only adv,can't connect
#define DEF_BLE_DEMO_STREAMER             2
#define DEF_BLE_DEMO_TRANS_DATA           3 //
#define DEF_BLE_DEMO_DUEROS_DMA           4 //
#define DEF_BLE_DEMO_RCSP_DEMO            5 //
#define DEF_BLE_DEMO_ADV_RCSP             6
#define DEF_BLE_DEMO_GMA                  7
#define DEF_BLE_DEMO_CLIENT               8 //


//配置选择的demo
#if TCFG_USER_BLE_ENABLE

#if DUEROS_DMA_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_DUEROS_DMA

#elif (RCSP_BTMATE_EN | SMART_BOX_EN)
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_RCSP_DEMO

#elif TRANS_DATA_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_TRANS_DATA

#elif RCSP_ADV_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_ADV_RCSP

#elif GMA_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_GMA

#elif BLE_CLIENT_EN
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_CLIENT

#else
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_ADV
#endif

#else
#define TCFG_BLE_DEMO_SELECT          DEF_BLE_DEMO_NULL//ble is closed
#endif

//配对加密使能
#define TCFG_BLE_SECURITY_EN          0



#endif
