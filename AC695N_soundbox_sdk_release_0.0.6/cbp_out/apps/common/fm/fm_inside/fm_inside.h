#ifndef _FM_INSIDE_H_
#define _FM_INSIDE_H_

#if(TCFG_FM_INSIDE_ENABLE == ENABLE)

#if DAC2IIS_EN
#define FM_DAC_OUT_SAMPLERATE  44100L
#else
#define FM_DAC_OUT_SAMPLERATE  44100L
#endif

/************************************************************
*                       FM调试说明
*真台少：                假台多：               叠台多： 
*减小  FMSCAN_CNR        加大  FMSCAN_CNR       减小 FMSCAN_AGC 
*减小  FMSCAN_P_DIFFER   加大  FMSCAN_P_DIFFER
*加大  FMSCAN_N_DIFFER   减小  FMSCAN_N_DIFFER
*
*注意：不要插着串口测试搜台数。
*************************************************************/

#define FMSCAN_SEEK_CNT_MIN  400 //最小过零点数 400左右
#define FMSCAN_SEEK_CNT_MAX  600 //最大过零点数 600左右
#define FMSCAN_960_CNR       34  //谐波96M的基础cnr 30~40
#define FMSCAN_1080_CNR      34  //谐波108M的基础cnr 30~40
#define FMSCAN_AGC 			 -57 //AGC阈值  -57左右
#define FMSCAN_ADD_DIFFER 	 -67 //低于此值增加noise differ -67左右  

#define FMSCAN_CNR           2   //cnr  1以上 
#define FMSCAN_P_DIFFER		 2   //power differ  1以上
#define FMSCAN_N_DIFFER   	 8   //noise differ  8左右

/* struct FM_INSIDE_DAT {
    AUDIO_STREAM *stream_io;
    volatile u8 src_toggle;
}; */



#endif

#endif // _FM_INSIDE_H_
