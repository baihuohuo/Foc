#ifndef __SYS_H
#define __SYS_H

#include "stm32f4xx.h"

// 系统是否支持操作系统 (0=不支持, 1=支持RTOS)
#define SYSTEM_SUPPORT_OS  0

// ==============================================================
// 位带操作 (Bit-band Alias)
// 实现51单片机类似的IO口位操作功能
// 参考: ARM Cortex-M4 权威指南
// ==============================================================
#define BITBAND(addr, bitnum) ((addr & 0xF0000000) + 0x2000000 + ((addr & 0xFFFFF) << 5) + (bitnum << 2))
#define MEM_ADDR(addr)       (*((volatile unsigned long *)(addr)))
#define BIT_ADDR(addr, bitnum)  MEM_ADDR(BITBAND(addr, bitnum))

// GPIO ODR (输出数据寄存器) 地址映射
#define GPIOA_ODR_Addr  (GPIOA_BASE + 20)
#define GPIOB_ODR_Addr  (GPIOB_BASE + 20)
#define GPIOC_ODR_Addr  (GPIOC_BASE + 20)
#define GPIOD_ODR_Addr  (GPIOD_BASE + 20)
#define GPIOE_ODR_Addr  (GPIOE_BASE + 20)
#define GPIOF_ODR_Addr  (GPIOF_BASE + 20)
#define GPIOG_ODR_Addr  (GPIOG_BASE + 20)
#define GPIOH_ODR_Addr  (GPIOH_BASE + 20)
#define GPIOI_ODR_Addr  (GPIOI_BASE + 20)

// GPIO IDR (输入数据寄存器) 地址映射
#define GPIOA_IDR_Addr  (GPIOA_BASE + 16)
#define GPIOB_IDR_Addr  (GPIOB_BASE + 16)
#define GPIOC_IDR_Addr  (GPIOC_BASE + 16)
#define GPIOD_IDR_Addr  (GPIOD_BASE + 16)
#define GPIOE_IDR_Addr  (GPIOE_BASE + 16)
#define GPIOF_IDR_Addr  (GPIOF_BASE + 16)
#define GPIOG_IDR_Addr  (GPIOG_BASE + 16)
#define GPIOH_IDR_Addr  (GPIOH_BASE + 16)
#define GPIOI_IDR_Addr  (GPIOI_BASE + 16)

// ==============================================================
// GPIO 位操作宏 (n: 0-15)
// ==============================================================
#define PAout(n)  BIT_ADDR(GPIOA_ODR_Addr, n)   // GPIOA 输出
#define PAin(n)   BIT_ADDR(GPIOA_IDR_Addr, n)   // GPIOA 输入

#define PBout(n)  BIT_ADDR(GPIOB_ODR_Addr, n)   // GPIOB 输出
#define PBin(n)   BIT_ADDR(GPIOB_IDR_Addr, n)   // GPIOB 输入

#define PCout(n)  BIT_ADDR(GPIOC_ODR_Addr, n)   // GPIOC 输出
#define PCin(n)   BIT_ADDR(GPIOC_IDR_Addr, n)   // GPIOC 输入

#define PDout(n)  BIT_ADDR(GPIOD_ODR_Addr, n)   // GPIOD 输出
#define PDin(n)   BIT_ADDR(GPIOD_IDR_Addr, n)   // GPIOD 输入

#define PEout(n)  BIT_ADDR(GPIOE_ODR_Addr, n)   // GPIOE 输出
#define PEin(n)   BIT_ADDR(GPIOE_IDR_Addr, n)   // GPIOE 输入

#define PFout(n)  BIT_ADDR(GPIOF_ODR_Addr, n)   // GPIOF 输出
#define PFin(n)   BIT_ADDR(GPIOF_IDR_Addr, n)   // GPIOF 输入

#define PGout(n)  BIT_ADDR(GPIOG_ODR_Addr, n)   // GPIOG 输出
#define PGin(n)   BIT_ADDR(GPIOG_IDR_Addr, n)   // GPIOG 输入

#define PHout(n)  BIT_ADDR(GPIOH_ODR_Addr, n)   // GPIOH 输出
#define PHin(n)   BIT_ADDR(GPIOH_IDR_Addr, n)   // GPIOH 输入

#define PIout(n)  BIT_ADDR(GPIOI_ODR_Addr, n)   // GPIOI 输出
#define PIin(n)   BIT_ADDR(GPIOI_IDR_Addr, n)   // GPIOI 输入

// ==============================================================
// 汇编函数声明 (Thumb指令集内联汇编)
// ==============================================================
void WFI_SET(void);        // 执行 WFI 指令 (进入低功耗)
void INTX_DISABLE(void);   // 关闭所有中断 (不包括 fault 和 NMI)
void INTX_ENABLE(void);    // 开启所有中断
void MSR_MSP(u32 addr);    // 设置主堆栈指针地址

#endif
