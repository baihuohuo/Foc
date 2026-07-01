#ifndef __BSP_ENCODER_H__                        
#define __BSP_ENCODER_H__                         

#include "stm32f4xx.h"                           

//TIM_EncoderMode_TI12 可以根据 A/B 两相的状态变化计数，所以一个脉冲周期不是只数 1 次，而是最多数 4 次。因而 TIM编码器模式四倍频后。

/* -----------------------宏定义 -------------------*/
#define BSP_ENCODER_AB_GPIO_CLK             RCC_AHB1Periph_GPIOC     // A/B相GPIO时钟
#define BSP_ENCODER_Z_GPIO_CLK              RCC_AHB1Periph_GPIOE     // Z相GPIO时钟
#define BSP_ENCODER_TIM_CLK                 RCC_APB1Periph_TIM3      // 编码器定时器时钟

#define BSP_ENCODER_TIM                     TIM3                     // 编码器使用TIM3
#define BSP_ENCODER_AF                      GPIO_AF_TIM3             // PC6/PC7复用为TIM3功能

#define BSP_ENCODER_A_GPIO_PORT             GPIOC                    // A相GPIO端口
#define BSP_ENCODER_A_GPIO_PIN              GPIO_Pin_6               // A相GPIO引脚
#define BSP_ENCODER_A_GPIO_PIN_SOURCE       GPIO_PinSource6          // A相GPIO复用引脚源

#define BSP_ENCODER_B_GPIO_PORT             GPIOC                    // B相GPIO端口
#define BSP_ENCODER_B_GPIO_PIN              GPIO_Pin_7               // B相GPIO引脚
#define BSP_ENCODER_B_GPIO_PIN_SOURCE       GPIO_PinSource7          // B相GPIO复用引脚源

#define BSP_ENCODER_Z_GPIO_PORT             GPIOE                    // Z相GPIO端口
#define BSP_ENCODER_Z_GPIO_PIN              GPIO_Pin_5               // Z相GPIO引脚

#define BSP_ENCODER_LINE_COUNT              256.0f                   // 编码器每机械圈脉冲数
#define BSP_ENCODER_QUADRATURE_COUNT        (BSP_ENCODER_LINE_COUNT * 4.0f) // 四倍频后每机械圈计数
#define BSP_ENCODER_POLE_PAIRS              4.0f                     // 电机极对数
#define BSP_ENCODER_ELECTRICAL_OFFSET_RAD   0.0f                     // 电角度零偏, 标定后填写
#define BSP_ENCODER_TWO_PI                  6.28318530718f           // 2*pi弧度

/* -----------------------类型定义 -------------------*/
typedef struct                                                  
{                                                                
    uint16_t RawCount;              // TIM3当前原始计数值, 0~65535循环
    uint16_t LastRawCount;          // 上一次采样的TIM3原始计数值
    int16_t DeltaCount;             // 当前采样周期内的计数变化量
    int32_t PositionCount;          // 累积位置计数, 可正可负

    float MechanicalAngleRad;       // 机械角度, 单位rad, 范围0~2pi
    float ElectricalAngleRad;       // 电角度, 单位rad, 范围0~2pi
    float MechanicalSpeedRadS;      // 机械角速度, 单位rad/s
    float ElectricalSpeedRadS;      // 电角速度, 单位rad/s

    uint8_t Direction;              // 方向标志: 0停止, 1正计数, 2负计数
    uint8_t ZState;                 // Z相当前输入电平
} BSP_EncoderDataTypeDef;                                     

/* -----------------------函数声明 -------------------*/
void BSP_Encoder_Init(void);                                    // 初始化编码器GPIO和TIM3
void BSP_Encoder_Update(float dt_sec);                          // 周期更新编码器角度和速度
void BSP_Encoder_SetZero(void);                                 // 清零当前位置和速度数据
uint8_t BSP_Encoder_ReadZ(void);                                // 读取Z相当前电平
float BSP_Encoder_GetMechanicalAngleRad(void);                  // 获取机械角度
float BSP_Encoder_GetElectricalAngleRad(void);                  // 获取电角度
float BSP_Encoder_GetMechanicalSpeedRadS(void);                 // 获取机械角速度
float BSP_Encoder_GetElectricalSpeedRadS(void);                 // 获取电角速度
BSP_EncoderDataTypeDef BSP_Encoder_GetAllData(void);            // 获取编码器全部运行数据

#endif 