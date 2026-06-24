#include "bsp_adc.h"
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_gpio.h"

/* -----------------------私有变量 -------------------*/
static uint16_t bsp_adc_raw_buffer[BSP_ADC_SAMPLE_COUNT];
static BSP_ADC_DataTypeDef bsp_adc_data;
static float mt2_amp_u_offset_voltage = BSP_ADC_MT2_AMP_ZERO_VOLTAGE;
static float mt2_amp_v_offset_voltage = BSP_ADC_MT2_AMP_ZERO_VOLTAGE;
static float mt2_amp_w_offset_voltage = BSP_ADC_MT2_AMP_ZERO_VOLTAGE;

/* -----------------------私有函数 -------------------*/
static float BSP_ADC_RawToPinVoltage(uint16_t raw)
{
    return ((float)raw * BSP_ADC_REFERENCE_VOLTAGE) / BSP_ADC_RESOLUTION_COUNT;
}

static float BSP_ADC_RawToPhaseCurrent_A(uint16_t raw, float offset_voltage)
{
    float pin_voltage;

    pin_voltage = BSP_ADC_RawToPinVoltage(raw);

    return (pin_voltage - offset_voltage) * BSP_ADC_MT2_AMP_VOLTAGE_SCALE;
}

// 校准延时
static void BSP_ADC_CalibrationDelay(void)
{
    volatile uint32_t delay_count;

    delay_count = 1000u;
    while (delay_count > 0u)
    {
        delay_count--;
    }
}

static float BSP_ADC_AbsFloat(float value)
{
    if (value < 0.0f)
    {
        return -value;
    }

    return value;
}

static uint16_t BSP_ADC_GetMax3(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t max_value;

    max_value = a;

    if (b > max_value)
    {
        max_value = b;
    }

    if (c > max_value)
    {
        max_value = c;
    }

    return max_value;
}

static float BSP_ADC_GetMaxAbs3(float a, float b, float c)
{
    float max_abs_value;
    float abs_value;

    max_abs_value = BSP_ADC_AbsFloat(a);

    abs_value = BSP_ADC_AbsFloat(b);
    if (abs_value > max_abs_value)
    {
        max_abs_value = abs_value;
    }

    abs_value = BSP_ADC_AbsFloat(c);
    if (abs_value > max_abs_value)
    {
        max_abs_value = abs_value;
    }

    return max_abs_value;
}

/* -----------------------公有函数 -------------------*/
void BSP_ADC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;

    /* 使能GPIO和DMA时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA |
                           RCC_AHB1Periph_GPIOB |
                           RCC_AHB1Periph_GPIOC |
                           RCC_AHB1Periph_DMA2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /* 配置ADC输入引脚为模拟模式 */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_3;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 配置DMA2_Stream0用于ADC数据搬运 */
    DMA_Cmd(DMA2_Stream0, DISABLE);
    while (DMA_GetCmdStatus(DMA2_Stream0) == ENABLE)
    {
    }
    DMA_DeInit(DMA2_Stream0);

    DMA_InitStructure.DMA_Channel = DMA_Channel_0;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)bsp_adc_raw_buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = BSP_ADC_SAMPLE_COUNT;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
    DMA_Cmd(DMA2_Stream0, ENABLE);

    /* 配置ADC公共参数 */
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);

    /* 配置ADC1参数: 12位, 扫描模式, 连续转换 */
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T8_TRGO;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = BSP_ADC_SAMPLE_COUNT;
    ADC_Init(ADC1, &ADC_InitStructure);


    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 1, ADC_SampleTime_84Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 2, ADC_SampleTime_84Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_9, 3, ADC_SampleTime_84Cycles);

    /* 使能ADC DMA请求, 启动ADC */
    ADC_DMARequestAfterLastTransferCmd(ADC1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);

}

void BSP_ADC_CalibrateMT2AmpOffset(void)
{
    uint32_t sample_index;
    float u_voltage_sum;
    float v_voltage_sum;
    float w_voltage_sum;

    u_voltage_sum = 0.0f;
    v_voltage_sum = 0.0f;
    w_voltage_sum = 0.0f;

    /* 采集多次ADC值取平均, 计算零点偏移 */
    for (sample_index = 0u; sample_index < BSP_ADC_MT2_AMP_CALIBRATION_TIMES; sample_index++)
    {
        BSP_ADC_CalibrationDelay();

        u_voltage_sum += BSP_ADC_RawToPinVoltage(bsp_adc_raw_buffer[BSP_ADC_INDEX_PHASE_MT2_AMPU]);
        v_voltage_sum += BSP_ADC_RawToPinVoltage(bsp_adc_raw_buffer[BSP_ADC_INDEX_PHASE_MT2_AMPV]);
        w_voltage_sum += BSP_ADC_RawToPinVoltage(bsp_adc_raw_buffer[BSP_ADC_INDEX_PHASE_MT2_AMPW]);
    }

    mt2_amp_u_offset_voltage = u_voltage_sum / (float)BSP_ADC_MT2_AMP_CALIBRATION_TIMES;
    mt2_amp_v_offset_voltage = v_voltage_sum / (float)BSP_ADC_MT2_AMP_CALIBRATION_TIMES;
    mt2_amp_w_offset_voltage = w_voltage_sum / (float)BSP_ADC_MT2_AMP_CALIBRATION_TIMES;

    BSP_ADC_Update();
}

void BSP_ADC_Update(void)
{
    /* 从DMA buffer读取原始ADC值 */
    bsp_adc_data.RawAdcPhaseMT2_AMPU = bsp_adc_raw_buffer[BSP_ADC_INDEX_PHASE_MT2_AMPU];
    bsp_adc_data.RawAdcPhaseMT2_AMPV = bsp_adc_raw_buffer[BSP_ADC_INDEX_PHASE_MT2_AMPV];
    bsp_adc_data.RawAdcPhaseMT2_AMPW = bsp_adc_raw_buffer[BSP_ADC_INDEX_PHASE_MT2_AMPW];
    bsp_adc_data.RawAdcBMT2_VBUS = 0u;
    bsp_adc_data.RawAdcMT2_Vtemp = 0u;

    /* 计算三相电流最大值 */
    bsp_adc_data.RawAdcPhaseMT2_I_MAX = BSP_ADC_GetMax3(bsp_adc_data.RawAdcPhaseMT2_AMPU,
                                                        bsp_adc_data.RawAdcPhaseMT2_AMPV,
                                                        bsp_adc_data.RawAdcPhaseMT2_AMPW);

    /* 将原始ADC值转换为物理量: 电流、电压、温度 */
    bsp_adc_data.RealPhaseMT2_AMPU = BSP_ADC_RawToPhaseCurrent_A(bsp_adc_data.RawAdcPhaseMT2_AMPU, mt2_amp_u_offset_voltage);
    bsp_adc_data.RealPhaseMT2_AMPV = BSP_ADC_RawToPhaseCurrent_A(bsp_adc_data.RawAdcPhaseMT2_AMPV, mt2_amp_v_offset_voltage);
    bsp_adc_data.RealPhaseMT2_AMPW = BSP_ADC_RawToPhaseCurrent_A(bsp_adc_data.RawAdcPhaseMT2_AMPW, mt2_amp_w_offset_voltage);
    bsp_adc_data.RealBMT2_VBUS = 0;
    bsp_adc_data.RealMT2_Vtemp = 0;

    /* 计算三相电流绝对值的最大值 */
    bsp_adc_data.RealPhaseMT2_I_MAX = BSP_ADC_GetMaxAbs3(bsp_adc_data.RealPhaseMT2_AMPU,
                                                        bsp_adc_data.RealPhaseMT2_AMPV,
                                                        bsp_adc_data.RealPhaseMT2_AMPW);
}


// 获取母线电压 (单位: V)
float BSP_ADC_GetRealBMT2_VBUS_Voltage(void)
{
    return bsp_adc_data.RealBMT2_VBUS;
}

float BSP_ADC_GetRealPhase_MT2_I_MAX(void)
{
    return bsp_adc_data.RealPhaseMT2_I_MAX;
}

// 获取完整ADC数据
BSP_ADC_DataTypeDef BSP_ADC_GetAllData(void)
{
    return bsp_adc_data;
}
