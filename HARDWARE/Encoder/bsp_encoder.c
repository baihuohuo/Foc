/* -----------------------编码器驱动 -------------------*/
#include "bsp_encoder.h"                         // 编码器驱动头文件
#include "stm32f4xx_gpio.h"                    
#include "stm32f4xx_rcc.h"                      
#include "stm32f4xx_tim.h"                      

static BSP_EncoderDataTypeDef bsp_encoder_data;  // 编码器数据

// 角度归一化: 将角度限制到0~2pi
static float BSP_Encoder_NormalizeAngle(float angle_rad)         // 输入任意弧度角, 返回0~2pi角度
{                                                               
    while (angle_rad >= BSP_ENCODER_TWO_PI)                     
    {                                                           
        angle_rad -= BSP_ENCODER_TWO_PI;                        
    }                                                           

    while (angle_rad < 0.0f)                                     
    {                                                            
        angle_rad += BSP_ENCODER_TWO_PI;                       
    }                                                           
    return angle_rad;                                            
}                                                               

// 计数取模: 将累积计数转换到单圈范围
static int32_t BSP_Encoder_ModCount(int32_t value, int32_t mod)  // 输入累积计数和单圈计数
{                                                                // 函数开始
    int32_t result;                                              // 保存取模结果

    result = value % mod;                                        // C语言取模, 负数结果可能仍为负
    if (result < 0)                                              // 如果取模结果为负
    {                                                            // 负数修正开始
        result += mod;                                           // 加上单圈计数, 修正到0~mod
    }                                                            // 负数修正结束

    return result;                                               // 返回单圈内计数
}                                                                // 函数结束

// GPIO初始化: PC6/PC7配置为TIM3输入, PE5配置为Z相输入
static void BSP_Encoder_GPIO_Init(void)                         
{                                                                
    GPIO_InitTypeDef GPIO_InitStructure;                        

    RCC_AHB1PeriphClockCmd(BSP_ENCODER_AB_GPIO_CLK | BSP_ENCODER_Z_GPIO_CLK, ENABLE); 

    GPIO_InitStructure.GPIO_Pin = BSP_ENCODER_A_GPIO_PIN | BSP_ENCODER_B_GPIO_PIN;   
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;                 // A/B相配置为复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;              
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           
    GPIO_Init(BSP_ENCODER_A_GPIO_PORT, &GPIO_InitStructure);    

    GPIO_PinAFConfig(BSP_ENCODER_A_GPIO_PORT, BSP_ENCODER_A_GPIO_PIN_SOURCE, BSP_ENCODER_AF); // PC6复用为TIM3_CH1
    GPIO_PinAFConfig(BSP_ENCODER_B_GPIO_PORT, BSP_ENCODER_B_GPIO_PIN_SOURCE, BSP_ENCODER_AF); // PC7复用为TIM3_CH2

    //Z 相是编码器的“零位脉冲”，Z 相通常每机械转一圈只出现一次，用来找机械零点。
    GPIO_InitStructure.GPIO_Pin = BSP_ENCODER_Z_GPIO_PIN;       
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;                 // Z相配置为普通输入
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;              
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;                 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;           
    GPIO_Init(BSP_ENCODER_Z_GPIO_PORT, &GPIO_InitStructure);    
}                                                               

// TIM3初始化: 使用编码器接口模式读取A/B相
static void BSP_Encoder_TIM_Init(void)                          
{                                                                
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;          
    TIM_ICInitTypeDef TIM_ICInitStructure;                       

    RCC_APB1PeriphClockCmd(BSP_ENCODER_TIM_CLK, ENABLE);        

    TIM_TimeBaseInitStructure.TIM_Prescaler = 0u;                // 不分频, 编码器直接计数
    TIM_TimeBaseInitStructure.TIM_Period = 0xFFFFu;              // 16位最大计数周期
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0u;        
    TIM_TimeBaseInit(BSP_ENCODER_TIM, &TIM_TimeBaseInitStructure);

    TIM_EncoderInterfaceConfig(BSP_ENCODER_TIM,                  // 配置TIM3为编码器接口模式
                               TIM_EncoderMode_TI12,            // TI1和TI2同时参与计数
                               TIM_ICPolarity_Rising,           // CH1上升沿极性
                               TIM_ICPolarity_Rising);          // CH2上升沿极性

    TIM_ICStructInit(&TIM_ICInitStructure);                      // 输入捕获结构体恢复默认值
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;             // 选择CH1输入
    TIM_ICInitStructure.TIM_ICFilter = 6u;                      
    TIM_ICInit(BSP_ENCODER_TIM, &TIM_ICInitStructure);           // 初始化CH1滤波参数

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;             // 选择CH2输入
    TIM_ICInitStructure.TIM_ICFilter = 6u;                      
    TIM_ICInit(BSP_ENCODER_TIM, &TIM_ICInitStructure);           // 初始化CH2滤波参数

    TIM_SetCounter(BSP_ENCODER_TIM, 0u);                         // TIM3计数器清零
    TIM_Cmd(BSP_ENCODER_TIM, ENABLE);                            // 使能TIM3
}                                                               

/* -----------------------对外函数 -------------------*/
// 编码器初始化
void BSP_Encoder_Init(void)                                      
{                                                                
    BSP_Encoder_GPIO_Init();                                     // 初始化A/B/Z相GPIO
    BSP_Encoder_TIM_Init();                                      // 初始化TIM3编码器模式

    bsp_encoder_data.RawCount = 0u;                             
    bsp_encoder_data.LastRawCount = 0u;                         
    bsp_encoder_data.DeltaCount = 0;                             
    bsp_encoder_data.PositionCount = 0;                        
    bsp_encoder_data.MechanicalAngleRad = 0.0f;                 
    bsp_encoder_data.ElectricalAngleRad = 0.0f;                  
    bsp_encoder_data.MechanicalSpeedRadS = 0.0f;                 
    bsp_encoder_data.ElectricalSpeedRadS = 0.0f;
    bsp_encoder_data.Direction = 0u;                            
    bsp_encoder_data.ZState = BSP_Encoder_ReadZ();              
}                                                               

// 按采样周期更新编码器数据
void BSP_Encoder_Update(float dt_sec)                           
{
    int32_t count_in_one_rev;                                    // 单机械圈总计数
    int32_t position_in_one_rev;                                 // 单机械圈内当前位置计数
    float mechanical_angle;                                      // 临时机械角度变量

    if (dt_sec <= 0.0f) dt_sec = 0.001f;                                                                                                 

    bsp_encoder_data.RawCount = TIM_GetCounter(BSP_ENCODER_TIM); // 读取TIM3当前原始计数

    bsp_encoder_data.DeltaCount = (int16_t)(bsp_encoder_data.RawCount - bsp_encoder_data.LastRawCount); // 计算本周期计数增量
    bsp_encoder_data.LastRawCount = bsp_encoder_data.RawCount;   // 保存本次计数作为下次参考
	
    bsp_encoder_data.PositionCount += (int32_t)bsp_encoder_data.DeltaCount; // 累加位置计数

    if (bsp_encoder_data.DeltaCount > 0)                          
    {                                                           
        bsp_encoder_data.Direction = 1u;                         // 方向标志置为正向
    }                                                           
    else if (bsp_encoder_data.DeltaCount < 0)                  
    {                                                           
        bsp_encoder_data.Direction = 2u;                         // 方向标志置为反向
    }                                                           
    else                                                        
    {                                                            
        bsp_encoder_data.Direction = 0u;                         // 方向标志置为停止
    }                                                            
    bsp_encoder_data.ZState = BSP_Encoder_ReadZ();               // 更新Z相当前电平

    count_in_one_rev = (int32_t)BSP_ENCODER_QUADRATURE_COUNT;    // 计算四倍频后的单圈计数
    position_in_one_rev = BSP_Encoder_ModCount(bsp_encoder_data.PositionCount, count_in_one_rev); // 换算单圈内计数

    mechanical_angle = ((float)position_in_one_rev * BSP_ENCODER_TWO_PI) / BSP_ENCODER_QUADRATURE_COUNT; // 由单圈计数换算机械角度

    bsp_encoder_data.MechanicalAngleRad = BSP_Encoder_NormalizeAngle(mechanical_angle); // 机械角度归一化

    bsp_encoder_data.ElectricalAngleRad = BSP_Encoder_NormalizeAngle((mechanical_angle * BSP_ENCODER_POLE_PAIRS) + BSP_ENCODER_ELECTRICAL_OFFSET_RAD); // 机械角度换算电角度并加入零偏
    
    bsp_encoder_data.MechanicalSpeedRadS = ((float)bsp_encoder_data.DeltaCount * BSP_ENCODER_TWO_PI) /
                                        (BSP_ENCODER_QUADRATURE_COUNT * dt_sec);

    bsp_encoder_data.ElectricalSpeedRadS = bsp_encoder_data.MechanicalSpeedRadS * BSP_ENCODER_POLE_PAIRS;

}    
                                                           

// 编码器清零
void BSP_Encoder_SetZero(void)                                  
{                                                               
    TIM_SetCounter(BSP_ENCODER_TIM, 0u);                         // TIM3硬件计数器清零

    bsp_encoder_data.RawCount = 0u;                             
    bsp_encoder_data.LastRawCount = 0u;                          
    bsp_encoder_data.DeltaCount = 0;                            
    bsp_encoder_data.PositionCount = 0;                         
    bsp_encoder_data.MechanicalAngleRad = 0.0f;                 
    bsp_encoder_data.ElectricalAngleRad = 0.0f;                 
    bsp_encoder_data.MechanicalSpeedRadS = 0.0f;                
    bsp_encoder_data.ElectricalSpeedRadS = 0.0f;                
}                                                             
// 读取Z相电平
uint8_t BSP_Encoder_ReadZ(void)                                  
{                                                                
    return (uint8_t)GPIO_ReadInputDataBit(BSP_ENCODER_Z_GPIO_PORT, BSP_ENCODER_Z_GPIO_PIN); // 读取PE5输入电平
}                                                               

// 获取机械角度rad
float BSP_Encoder_GetMechanicalAngleRad(void)                   
{                                                               
    return bsp_encoder_data.MechanicalAngleRad;                 
}                                                               

// 获取电角度rad
float BSP_Encoder_GetElectricalAngleRad(void)                    
{                                                               
    return bsp_encoder_data.ElectricalAngleRad;                  
}                                                               

 // 返回机械角速度rad/s
float BSP_Encoder_GetMechanicalSpeedRadS(void)                 
{                                                               
    return bsp_encoder_data.MechanicalSpeedRadS;                 
}                                                               

 // 返回电角速度rad/s
float BSP_Encoder_GetElectricalSpeedRadS(void)                  
{                                                               
    return bsp_encoder_data.ElectricalSpeedRadS;                
}                                                               

// 获取编码器全部数据
BSP_EncoderDataTypeDef BSP_Encoder_GetAllData(void)              
{                                                              
    return bsp_encoder_data;                                    
}                                                               
