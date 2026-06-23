/* -----------------------TIM8 PWM 驱动 -------------------*/
#include "bsp_tim8_pwm.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"


// TIM8 PWM模块一键初始化
void TIM8_PWM_ModuleInit(void)
{
    TIM8_PWM_GPIO_Init();
    TIM8_PWM_Init();
    TIM8_PWM_Start();
}
// GPIO初始化: PI5/6/7 主输出, PH13/14/15 互补输出, PH9 刹车输入
void TIM8_PWM_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH, ENABLE);

    // 主输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOI, &GPIO_InitStructure);

    // 互补输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOH, &GPIO_InitStructure);

    // 刹车输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOH, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOI, GPIO_PinSource5, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOI, GPIO_PinSource6, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOI, GPIO_PinSource7, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource13, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource14, GPIO_AF_TIM8);
    GPIO_PinAFConfig(GPIOH, GPIO_PinSource15, GPIO_AF_TIM8);
}

// PWM初始化: 20kHz中心对齐, 带死区, 三相互补输出
void TIM8_PWM_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_BDTRInitTypeDef TIM_BDTRInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);

    // 定时器时基单元
    // PWM频率计算 (中心对齐模式3):
    //   f_PWM = TIM8_CLK / (2 × (Period + 1) × (Prescaler + 1))
    //         = 168MHz / (2 × 4200 × 1) = 20 kHz
    TIM_TimeBaseInitStructure.TIM_Prescaler = 0;
    TIM_TimeBaseInitStructure.TIM_Period = 4200-1;                // ARR = 4200-1
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned3;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 1;
    TIM_TimeBaseInit(TIM8, &TIM_TimeBaseInitStructure);

    // PWM输出通道配置: PWM1模式, 计数值<CCR为高
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;              // 计数值 < CCR 输出高
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;   // 空闲状态
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;      // 主输出有效电平为高
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = 0;                             // 初始占空比为0
    TIM_OC1Init(TIM8,&TIM_OCInitStructure);
    TIM_OC2Init(TIM8,&TIM_OCInitStructure);
    TIM_OC3Init(TIM8,&TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM8, ENABLE);
	
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
	TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
	TIM_OCInitStructure.TIM_Pulse = 4200u - 500u;
	TIM_OC4Init(TIM8, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM8, TIM_OCPreload_Enable);
    TIM_SelectOutputTrigger(TIM8, TIM_TRGOSource_OC4Ref);

    // 死区与刹车配置
    TIM_BDTRInitStructure.TIM_OSSRState = TIM_OSSRState_Enable;    // 运行模式关闭状态强制无效电平
    TIM_BDTRInitStructure.TIM_OSSIState = TIM_OSSIState_Enable;    // 空闲模式关闭状态强制无效电平
    TIM_BDTRInitStructure.TIM_LOCKLevel = TIM_LOCKLevel_OFF;
    TIM_BDTRInitStructure.TIM_DeadTime = 84;
    TIM_BDTRInitStructure.TIM_Break = TIM_Break_Disable;          // 刹车功能总开关
    TIM_BDTRInitStructure.TIM_BreakPolarity = TIM_BreakPolarity_High;  // 刹车有效极性
    TIM_BDTRInitStructure.TIM_AutomaticOutput = TIM_AutomaticOutput_Disable;  // 自动输出恢复
    TIM_BDTRConfig(TIM8, &TIM_BDTRInitStructure);
}

// 启动PWM输出
void TIM8_PWM_Start(void)
{
    TIM_SetCompare1(TIM8, 0);
    TIM_SetCompare2(TIM8, 0);
    TIM_SetCompare3(TIM8, 0);

    TIM_ClearFlag(TIM8, TIM_FLAG_Update);
    TIM_CtrlPWMOutputs(TIM8, ENABLE);
    TIM_Cmd(TIM8, ENABLE);
}

// 停止PWM输出
void TIM8_PWM_Stop(void)
{
    TIM_CtrlPWMOutputs(TIM8, DISABLE);

    TIM_SetCompare1(TIM8, 0);
    TIM_SetCompare2(TIM8, 0);
    TIM_SetCompare3(TIM8, 0);

}

// 设置三相占空比 (带上限保护)
void TIM8_PWM_SetDuty(uint16_t duty_a, uint16_t duty_b, uint16_t duty_c)
{
    uint16_t period;

    period = TIM8_PWM_GetPeriod();

    if (duty_a > period) duty_a = period;
    if (duty_b > period) duty_b = period;
    if (duty_c > period) duty_c = period;

    TIM_SetCompare1(TIM8, duty_a);
    TIM_SetCompare2(TIM8, duty_b);
    TIM_SetCompare3(TIM8, duty_c);
}

// 获取PWM周期计数值
uint16_t TIM8_PWM_GetPeriod(void)
{
    return (uint16_t)TIM8->ARR;
}
