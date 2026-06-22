#include "bsp_vofa.h"
#include "usart.h"

/**
  * 函数功能: 发送数据到VOFA+ (JustFloat协议)
  * @param     ch1-ch3: 要发送的3个浮点数据
  * @note      CH0=当前转速(RPM), CH1=目标转速(RPM), CH2=占空比
  */
void VOFA_SendJustFloat(float ch1, float ch2, float ch3)
{
    uint8_t i;
    uint8_t txData[16];

    /* 填充3个float数据 */
    uint8_t *pFloat = (uint8_t *)&ch1;
    for(i = 0; i < 4; i++)  txData[i] = pFloat[i];

    pFloat = (uint8_t *)&ch2;
    for(i = 0; i < 4; i++)  txData[4+i] = pFloat[i];

    pFloat = (uint8_t *)&ch3;
    for(i = 0; i < 4; i++)  txData[8+i] = pFloat[i];

    /* 帧尾: 00 00 80 7F (0x7F800000 float的无穷大标记) */
    txData[12] = 0x00;
    txData[13] = 0x00;
    txData[14] = 0x80;
    txData[15] = 0x7F;

    /* 发送 */
    for(i = 0; i < 16; i++)
    {
        while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, txData[i]);
    }
}

