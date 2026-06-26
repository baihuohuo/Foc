#ifndef __BSP_HALL_H__
#define __BSP_HALL_H__

#include "stm32f4xx.h"

/* -----------------------宏定义 -------------------*/
#define BSP_HALL_U_GPIO_PORT              GPIOH
#define BSP_HALL_U_GPIO_PIN               GPIO_Pin_10
#define BSP_HALL_U_GPIO_CLK               RCC_AHB1Periph_GPIOH

#define BSP_HALL_V_GPIO_PORT              GPIOH
#define BSP_HALL_V_GPIO_PIN               GPIO_Pin_11
#define BSP_HALL_V_GPIO_CLK               RCC_AHB1Periph_GPIOH

#define BSP_HALL_W_GPIO_PORT              GPIOH
#define BSP_HALL_W_GPIO_PIN               GPIO_Pin_12
#define BSP_HALL_W_GPIO_CLK               RCC_AHB1Periph_GPIOH


#define BSP_HALL_POLE_PAIRS                 4u          /* 电机极对数 */
#define BSP_HALL_STATE_MASK                 0x07u       /* Hall 三位状态 */
#define BSP_HALL_INVALID_STATE              0u          /* 非法 Hall 状态标记 */
#define BSP_HALL_DEFAULT_ANGLE_OFFSET       0.0f        /* 默认电角度偏置, 单位: rad */
#define BSP_HALL_STEP_ANGLE_RAD           1.0471976f    /* Hall 每跳变一次对应 60 电角度, pi / 3 */
#define BSP_HALL_TWO_PI                   6.2831853f    /* 2 * pi */

/* -----------------------类型定义 -------------------*/
typedef enum
{
    BSP_HALL_DIR_UNKNOWN = 0,     // 方向未知或未运动 
    BSP_HALL_DIR_FORWARD = 1,     // Hall 状态按正方向变化 
    BSP_HALL_DIR_REVERSE = 2      // Hall 状态按反方向变化 
} BSP_HALL_DirectionTypeDef;

typedef struct
{
    uint8_t HallU;                // Hall U 相电平, 0 或 1 
    uint8_t HallV;                // Hall V 相电平, 0 或 1 
    uint8_t HallW;                // Hall W 相电平, 0 或 1 
    uint8_t HallState;            // 三相 Hall 合成状态: bit2=U, bit1=V, bit0=W 
    uint8_t LastHallState;        // 上一次有效 Hall 状态 

    float RawElectricalAngle;     // 离散电角度, 单位: rad 
    float ElectricalAngle;        // 当前电角度, 单位: rad 
    float AngleOffset;            // 电角度偏置, 单位: rad 

    float ElectricalSpeedRadS;     // 估算电角速度, 单位: rad/s 
    float TimeFromLastEdgeSec;     // 距离上一次 Hall 跳变的时间, 单位: s 
    float LastEdgePeriodSec;       // 上一次 Hall 跳变周期, 单位: s 

    BSP_HALL_DirectionTypeDef Direction;  // 当前估计方向 
} BSP_HALL_DataTypeDef;

/* -----------------------函数声明 -------------------*/
void BSP_HALL_Init(void);                                      // Hall GPIO 初始化
void BSP_HALL_Update(float dt_sec);                                    // 更新 Hall 状态和电角度

uint8_t BSP_HALL_ReadState(void);                              // 读取三相 Hall 合成状态
uint8_t BSP_HALL_IsValidState(uint8_t hall_state);             // 判断 Hall 状态是否有效

float BSP_HALL_StateToElectricalAngle(uint8_t hall_state);     // Hall 状态转换为离散电角度, 单位: rad 
void BSP_HALL_SetAngleOffset(float angle_offset);              // 设置电角度偏置, 单位: rad 

float BSP_HALL_GetRawElectricalAngle(void);                    // 获取 Hall 离散电角度, 单位: rad 
float BSP_HALL_GetElectricalAngle(void);                       // 获取连续电角度, 单位: rad 
float BSP_HALL_GetElectricalSpeedRadS(void);                   // 获取估算电角速度, 单位: rad/s 

BSP_HALL_DirectionTypeDef BSP_HALL_GetDirection(void);         // 获取当前 Hall 方向
BSP_HALL_DataTypeDef BSP_HALL_GetAllData(void);                // 获取完整 Hall 数据
#endif /* __BSP_HALL_H__ */
