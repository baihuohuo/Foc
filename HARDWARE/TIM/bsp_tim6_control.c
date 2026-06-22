/* -----------------------TIM6 控制周期定时器 -------------------*/
#include "bsp_tim6_control.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "misc.h"
#include "open_loop_foc.h"

// 记录当前控制周期, 单位: ms
static uint16_t tim6_control_period_ms = TIM6_CONTROL_DEFAULT_PERIOD_MS;


// 限制周期在合法范围内
static uint16_t TIM6_Control_LimitPeriod(uint16_t period_ms)
{
    if (period_ms < TIM6_CONTROL_MIN_PERIOD_MS)
    {
        return TIM6_CONTROL_MIN_PERIOD_MS;
    }
    if (period_ms > TIM6_CONTROL_MAX_PERIOD_MS)
    {
        return TIM6_CONTROL_MAX_PERIOD_MS;
    }
    return period_ms;
}

// 初始化TIM6: APB1=84MHz, 预分频8400得到10kHz计数频率, 10次计数=1ms, ARR=period_ms*10-1
void TIM6_Control_Init(uint16_t period_ms)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    tim6_control_period_ms = TIM6_Control_LimitPeriod(period_ms);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

    // 时基配置: 10kHz计数频率, 每period_ms毫秒触发一次更新中断
    TIM_TimeBaseInitStructure.TIM_Prescaler = 8400u - 1u;            // 预分频 8400, 得到 10kHz
    TIM_TimeBaseInitStructure.TIM_Period = (uint16_t)(tim6_control_period_ms * 10u - 1u);  // ARR = period_ms * 10 - 1
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0u;
    TIM_TimeBaseInit(TIM6, &TIM_TimeBaseInitStructure);

    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1u;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1u;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 默认不使能计数, 由 Start 函数启动
    TIM_Cmd(TIM6, DISABLE);
}

// 启动TIM6计数
void TIM6_Control_Start(void)
{
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
    TIM_Cmd(TIM6, ENABLE);
}

// 停止TIM6计数
void TIM6_Control_Stop(void)
{
    TIM_Cmd(TIM6, DISABLE);
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}

// 设置控制周期: 立即更新ARR, 同步触发一次更新事件
void TIM6_Control_SetPeriod(uint16_t period_ms)
{
    tim6_control_period_ms = TIM6_Control_LimitPeriod(period_ms);

    TIM_SetAutoreload(TIM6, (uint16_t)(tim6_control_period_ms * 10u - 1u));
    TIM_GenerateEvent(TIM6, TIM_EventSource_Update);
    TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}

// 获取当前控制周期 (单位: ms)
uint16_t TIM6_Control_GetPeriod(void)
{
    return tim6_control_period_ms;
}

// TIM6更新中断服务函数: 每次触发调用一次FOC更新
void TIM6_DAC_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        OpenLoopFOC_Update();
    }
}
