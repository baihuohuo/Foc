#include "sys.h"

// ==============================================================
// 汇编函数实现
// 注意: Thumb指令不支持内联汇编,需使用 __asm 关键字
// ==============================================================

// 进入睡眠模式 (Wait For Interrupt)
__asm void WFI_SET(void)
{
    WFI
}

// 关闭所有中断 (Primask)
__asm void INTX_DISABLE(void)
{
    CPSID   I
    BX      LR
}

// 开启所有中断
__asm void INTX_ENABLE(void)
{
    CPSIE   I
    BX      LR
}

// 设置主堆栈指针 (MSP)
// addr: 堆栈顶地址
__asm void MSR_MSP(u32 addr)
{
    MSR MSP, r0
    BX r14
}
