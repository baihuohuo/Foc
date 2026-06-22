#ifndef __BSP_VOFA_H__
#define __BSP_VOFA_H__

#include "stm32f4xx.h"

/* VOFA+ JustFloat 协议帧尾标识 */
#define VOFA_TAIL_MAGIC  0x7F800000u

/**
  * 函数功能: 发送数据到VOFA+ (JustFloat协议)
  * @param     ch1-ch3: 要发送的3个浮点数据
  * @note      CH0=当前转速(RPM), CH1=目标转速(RPM), CH2=占空比
  */
void VOFA_SendJustFloat(float ch1, float ch2, float ch3);

#endif
