#include "delay.h"
#include "sys.h"

// SysTick 时钟预分频系数 (固定为 AHB/8)
#define SYSTICK_PRERO  8

// fac_us: 微秒延时倍乘数 (每微秒需要的 SysTick 计数次数)
// fac_ms: 毫秒延时倍乘数 (每毫秒需要的 SysTick 计数次数)
static u8  fac_us = 0;
static u16 fac_ms = 0;

// ==============================================================
// 初始化延时函数
// 配置 SysTick 定时器,计算延时因子
// ==============================================================
void delay_init(u8 SYSCLK)
{
    // SysTick 时钟源 = AHB / 8
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    // 计算微秒延时因子
    fac_us = SYSCLK / SYSTICK_PRERO;

    // 计算毫秒延时因子
    // 注意: 这里用 fac_us * 1000, 可能在某些编译器下溢出
    // STM32F4 在 168MHz 时, fac_us = 21, 不会溢出
    fac_ms = (u16)fac_us * 1000;
}

// ==============================================================
// 微秒延时 (不使用RTOS)
// 使用 SysTick 倒计时实现精确延时
// ==============================================================
void delay_us(u32 nus)
{
    u32 temp;

    // 设置重装载值
    SysTick->LOAD = nus * fac_us;

    // 清空计数器
    SysTick->VAL = 0x00;

    // 启动倒计时
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // 等待计数完成 (COUNTFLAG 位置1时表示计数到0)
    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));

    // 关闭计数器
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    // 清空计数器
    SysTick->VAL = 0x00;
}

// ==============================================================
// 内部函数: 毫秒延时 (单次最大约 798ms @ 168MHz)
// ==============================================================
static void delay_xms(u16 nms)
{
    u32 temp;

    // 设置重装载值 (24位寄存器, 最大 16777215)
    SysTick->LOAD = (u32)nms * fac_ms;

    // 清空计数器
    SysTick->VAL = 0x00;

    // 启动倒计时
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // 等待计数完成
    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16)));

    // 关闭计数器
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    // 清空计数器
    SysTick->VAL = 0x00;
}

// ==============================================================
// 毫秒延时
// 通过多次调用 delay_xms 实现较长延时
// 拆分为 540ms 为单位,兼容超频到 248MHz 的情况
// ==============================================================
void delay_ms(u16 nms)
{
    u8 repeat = nms / 540;
    u16 remain = nms % 540;

    while (repeat--) {
        delay_xms(540);
    }

    if (remain) {
        delay_xms(remain);
    }
}
