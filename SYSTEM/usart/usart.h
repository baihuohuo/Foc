#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "stm32f4xx_conf.h"
#include "sys.h"

// ==============================================================
// 串口配置
// ==============================================================

// 接收缓冲区大小
#define USART_REC_LEN  200

// 使能串口接收中断 (1=使能, 0=禁止)
#define EN_USART1_RX    1

// 接收缓冲区 (最大 USART_REC_LEN 字节)
extern u8 USART_RX_BUF[USART_REC_LEN];

// 接收状态标志
// bit15:    接收完成标志 (1=完成)
// bit14:    接收到回车符 (0x0D)
// bit13~0:  已接收的字节数
extern u16 USART_RX_STA;

// 初始化串口
// bound: 波特率 (如 115200)
void uart_init(u32 bound);

#endif
