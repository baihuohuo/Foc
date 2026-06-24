#ifndef __BSP_ADC_H__
#define __BSP_ADC_H__

#include "stm32f4xx.h"

/* -----------------------宏定义 -------------------*/
#define BSP_ADC_REFERENCE_VOLTAGE          3.3f        // ADC参考电压, 单位: V
#define BSP_ADC_RESOLUTION_COUNT           4095.0f     // 12位ADC最大计数值
#define BSP_ADC_SAMPLE_COUNT               3u          // ADC1 DMA采样通道数量

/* DMA buffer下标, 必须和ADC规则通道Rank顺序一致 */
#define BSP_ADC_INDEX_PHASE_MT2_AMPU       0u          // PA6  ADC1_IN6  MT2_AMPU
#define BSP_ADC_INDEX_PHASE_MT2_AMPV       1u          // PB0  ADC1_IN8  MT2_AMPV
#define BSP_ADC_INDEX_PHASE_MT2_AMPW       2u          // PB1  ADC1_IN9  MT2_AMPW
#define BSP_ADC_INDEX_BMT2_VBUS            3u          // PC0  ADC1_IN10 MT2_VBUS
#define BSP_ADC_INDEX_MT2_Vtemp            4u          // PC3  ADC1_IN13 MT2_Vtemp

/* 母线电压分压比: (40.2K + 40.2K + 3.9K) / 3.9K */
#define BSP_ADC_BMT2_VBUS_VOLTAGE_SCALE    21.6153846f

/* 相电流采样参数: 0.02R采样电阻, 约4.02x放大倍数 */
#define BSP_ADC_MT2_AMP_ZERO_VOLTAGE       1.65f       // 零点偏置电压, 单位: V
#define BSP_ADC_MT2_AMP_SHUNT_RESISTOR     0.02f       // 采样电阻, 单位: Ω
#define BSP_ADC_MT2_AMP_GAIN               (5100.0f / (1200.0f + 68.0f))  // 放大器增益
#define BSP_ADC_MT2_AMP_VOLTAGE_SCALE      (1.0f / (BSP_ADC_MT2_AMP_SHUNT_RESISTOR * BSP_ADC_MT2_AMP_GAIN))
#define BSP_ADC_MT2_AMP_CALIBRATION_TIMES  3000u       // 零点校准采样次数

/* 温度: 第一版直接返回ADC引脚电压, 单位: V, 非摄氏度 */
#define BSP_ADC_MT2_Vtemp_SCALE            1.0f

/* -----------------------类型定义 -------------------*/
typedef enum
{
    BSP_ADC_CHANNEL_PHASE_MT2_AMPU = 0,   // U相电流采样通道
    BSP_ADC_CHANNEL_PHASE_MT2_AMPV,       // V相电流采样通道
    BSP_ADC_CHANNEL_PHASE_MT2_AMPW,       // W相电流采样通道
    BSP_ADC_CHANNEL_BMT2_VBUS,            // 直流母线电压采样通道
    BSP_ADC_CHANNEL_MT2_Vtemp,            // 温度采样通道
    BSP_ADC_CHANNEL_PHASE_MT2_I_MAX,      // 三相电流最大值, 用于过流保护
    BSP_ADC_CHANNEL_COUNT                 // ADC通道数量
} BSP_ADC_ChannelTypeDef;

typedef struct
{
    uint16_t RawAdcPhaseMT2_AMPU;          // U相电流原始ADC值
    uint16_t RawAdcPhaseMT2_AMPV;          // V相电流原始ADC值
    uint16_t RawAdcPhaseMT2_AMPW;          // W相电流原始ADC值
    uint16_t RawAdcBMT2_VBUS;              // 母线电压原始ADC值
    uint16_t RawAdcMT2_Vtemp;              // 温度原始ADC值
    uint16_t RawAdcPhaseMT2_I_MAX;         // U/V/W相电流原始值中的最大值

    float RealPhaseMT2_AMPU;               // U相真实电流, 单位: A
    float RealPhaseMT2_AMPV;               // V相真实电流, 单位: A
    float RealPhaseMT2_AMPW;               // W相真实电流, 单位: A
    float RealBMT2_VBUS;                   // 直流母线电压, 单位: V
    float RealMT2_Vtemp;                   // 温度ADC引脚电压, 单位: V
    float RealPhaseMT2_I_MAX;              // U/V/W相电流绝对值的最大值, 单位: A
} BSP_ADC_DataTypeDef;

/* -----------------------函数声明 -------------------*/
void BSP_ADC_Init(void);                                          // ADC初始化
void BSP_ADC_CalibrateMT2AmpOffset(void);                         // MT2相电流零点校准(电机断电时调用)
void BSP_ADC_Update(void);                                        // 更新ADC采样数据
float BSP_ADC_GetRealBMT2_VBUS_Voltage(void);                     // 获取母线电压 (单位: V)
float BSP_ADC_GetRealPhase_MT2_I_MAX(void);                       // 获取三相电流最大绝对值 (单位: A)
BSP_ADC_DataTypeDef BSP_ADC_GetAllData(void);                     // 获取完整ADC数据

//暂时不需要的函数
float BSP_ADC_GetMT2AmpU_OffsetVoltage(void);                     // 获取U相校准后的零点电压 (单位: V)
float BSP_ADC_GetMT2AmpV_OffsetVoltage(void);                     // 获取V相校准后的零点电压 (单位: V)
float BSP_ADC_GetMT2AmpW_OffsetVoltage(void);                     // 获取W相校准后的零点电压 (单位: V)

#endif /* __BSP_ADC_H__ */
