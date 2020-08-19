#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

/*
 *  板级配置选择
 */

#define CONFIG_BOARD_AC695X_DEMO
// #define CONFIG_BOARD_AC6955F_HEADSET_MONO
// #define CONFIG_BOARD_AC6952E_LIGHTER
 //#define CONFIG_BOARD_AC695X_CHARGING_BIN
// #define CONFIG_BOARD_AC695X_BTEMITTER
// #define CONFIG_BOARD_AC695X_TWS_BOX
// #define CONFIG_BOARD_AC695X_TWS
// #define CONFIG_BOARD_AC695X_MULTIMEDIA_CHARGING_BIN
// #define CONFIG_BOARD_AC695X_SOUNDCARD
// #define CONFIG_BOARD_AC6954A_DEMO



#define DAC_OUTPUT_MONO_L                  0    //左声道
#define DAC_OUTPUT_MONO_R                  1    //右声道
#define DAC_OUTPUT_LR                      2    //立体声
#define DAC_OUTPUT_MONO_LR_DIFF            3    //单声道差分输出

#define DAC_OUTPUT_DUAL_LR_DIFF            6    //双声道差分
#define DAC_OUTPUT_FRONT_LR_REAR_L         7    //三声道单端输出 前L+前R+后L (不可设置vcmo公共端)
#define DAC_OUTPUT_FRONT_LR_REAR_R         8    //三声道单端输出 前L+前R+后R (可设置vcmo公共端)
#define DAC_OUTPUT_FRONT_LR_REAR_LR        9    //四声道单端输出(不可设置vcmo公共端)

#include "board_ac6954a_demo_cfg.h"
#include "board_ac695x_demo_cfg.h"
#include "board_ac6952e_lighter_cfg.h"       //蓝牙点烟器
#include "board_ac6955f_headset_mono_cfg.h" //蓝牙带卡耳机
#include "board_ac695x_charging_bin.h"   //智能充电仓
#include "board_ac695x_btemitter.h"   //蓝牙发射器
#include "board_ac695x_tws_box.h"   //对箱
#include "board_ac695x_tws.h"   //纯对箱
#include "board_ac695x_multimedia_charging_bin.h"   //多媒体智能充电仓发射器
#include "board_ac695x_soundcard.h"   //声卡




#endif