#ifndef __BSP_KEY_H__
#define __BSP_KEY_H__

#include "stm32f4xx.h"

/* -----------------------类型定义 -------------------*/
typedef enum
{
  KEY_UP   = 0,    // 按键未按下
  KEY_DOWN = 1,    // 按键已按下
}KEYState_TypeDef;

/* -----------------------宏定义 -------------------*/
#define KEY1_RCC_CLOCKGPIO            RCC_AHB1Periph_GPIOE     // KEY1 GPIO时钟
#define KEY1_GPIO_PIN                 GPIO_Pin_0               // KEY1 GPIO引脚
#define KEY1_GPIO                     GPIOE                    // KEY1 GPIO端口
#define KEY1_DOWN_LEVEL               0                        // KEY1按下时为低电平

#define KEY2_RCC_CLOCKGPIO            RCC_AHB1Periph_GPIOE     // KEY2 GPIO时钟
#define KEY2_GPIO_PIN                 GPIO_Pin_1               // KEY2 GPIO引脚
#define KEY2_GPIO                     GPIOE                    // KEY2 GPIO端口
#define KEY2_DOWN_LEVEL               0                        // KEY2按下时为低电平

#define KEY3_RCC_CLOCKGPIO            RCC_AHB1Periph_GPIOE     // KEY3 GPIO时钟
#define KEY3_GPIO_PIN                 GPIO_Pin_2               // KEY3 GPIO引脚
#define KEY3_GPIO                     GPIOE                    // KEY3 GPIO端口
#define KEY3_DOWN_LEVEL               0                        // KEY3按下时为低电平

#define KEY4_RCC_CLOCKGPIO            RCC_AHB1Periph_GPIOE     // KEY4 GPIO时钟
#define KEY4_GPIO_PIN                 GPIO_Pin_3               // KEY4 GPIO引脚
#define KEY4_GPIO                     GPIOE                    // KEY4 GPIO端口
#define KEY4_DOWN_LEVEL               0                        // KEY4按下时为低电平

#define KEY5_RCC_CLOCKGPIO            RCC_AHB1Periph_GPIOE     // KEY5 GPIO时钟
#define KEY5_GPIO_PIN                 GPIO_Pin_4               // KEY5 GPIO引脚
#define KEY5_GPIO                     GPIOE                    // KEY5 GPIO端口
#define KEY5_DOWN_LEVEL               0                        // KEY5按下时为低电平

/* -----------------------函数声明 -------------------*/
void KEY_GPIO_Init(void);                     // 初始化GPIO (浮空输入)

// 阻塞型按键读取: 等待按键弹开后返回
KEYState_TypeDef KEY1_StateRead(void);        // 读取KEY1状态
KEYState_TypeDef KEY2_StateRead(void);        // 读取KEY2状态
KEYState_TypeDef KEY3_StateRead(void);        // 读取KEY3状态
KEYState_TypeDef KEY4_StateRead(void);        // 读取KEY4状态
KEYState_TypeDef KEY5_StateRead(void);        // 读取KEY5状态

// 非阻塞型边沿检测: 只在新按下瞬间返回KEY_DOWN
void KEY_EdgeStateReset(void);                // 重置边沿检测状态
KEYState_TypeDef KEY1_EdgeRead(void);         // 读取KEY1边沿
KEYState_TypeDef KEY2_EdgeRead(void);         // 读取KEY2边沿
KEYState_TypeDef KEY3_EdgeRead(void);         // 读取KEY3边沿
KEYState_TypeDef KEY4_EdgeRead(void);         // 读取KEY4边沿
KEYState_TypeDef KEY5_EdgeRead(void);         // 读取KEY5边沿

#endif /* __BSP_KEY_H__ */
