#include "stm32f4xx.h"
#include "usart.h"
#include "bsp_systick.h"
#include "bsp_key.h"
#include "bsp_tim8_pwm.h"
#include "bsp_tim6_control.h"
#include "bsp_adc.h"
#include "foc_math.h"
#include "open_loop_foc.h"
#include "bsp_vofa.h"

#define APP_UQ_STEP              0.2f
#define APP_SPEED_STEP_RAD_S     6.2831853f
#define APP_ADC_PRINT_PERIOD_MS  10u


static void App_PrintFOCState(void)
{
}

static void App_PrintADCState(void)
{
    BSP_ADC_DataTypeDef adc_data;

    adc_data = BSP_ADC_GetAllData();

    VOFA_SendJustFloat(adc_data.RealPhaseMT2_AMPU,
                       adc_data.RealPhaseMT2_AMPV,
                       adc_data.RealPhaseMT2_AMPW);
}

static void App_KeyTask(void)
{
    OpenLoopFOC_StateTypeDef state;
    float uq_ref;
    float speed_ref;

    state = OpenLoopFOC_GetState();
    uq_ref = state.UqRef;
    speed_ref = state.SpeedRefRadS;

    if (KEY1_EdgeRead() == KEY_DOWN)
    {
        if (OpenLoopFOC_GetIsRunning() != 0u)
        {
            OpenLoopFOC_Stop();
        }
        else
        {
            OpenLoopFOC_Start();
        }
        App_PrintFOCState();
    }

    if (KEY2_EdgeRead() == KEY_DOWN)
    {
        uq_ref += APP_UQ_STEP;
        OpenLoopFOC_SetTarget(uq_ref, speed_ref);
        App_PrintFOCState();
    }

    if (KEY3_EdgeRead() == KEY_DOWN)
    {
        uq_ref -= APP_UQ_STEP;
        OpenLoopFOC_SetTarget(uq_ref, speed_ref);
        App_PrintFOCState();
    }

    if (KEY4_EdgeRead() == KEY_DOWN)
    {
        speed_ref += APP_SPEED_STEP_RAD_S;
        OpenLoopFOC_SetTarget(uq_ref, speed_ref);
        App_PrintFOCState();
    }

    if (KEY5_EdgeRead() == KEY_DOWN)
    {
        speed_ref -= APP_SPEED_STEP_RAD_S;
        OpenLoopFOC_SetTarget(uq_ref, speed_ref);
        App_PrintFOCState();
    }
}

int main(void)
{
    uint16_t adc_print_tick = 0u;

    SysTick_Init();
    uart_init(115200);

    KEY_GPIO_Init();
    KEY_EdgeStateReset();

    TIM8_PWM_ModuleInit();
    BSP_ADC_Init();

    OpenLoopFOC_Init();
    OpenLoopFOC_Stop();

    BSP_DelayMs(100);
    BSP_ADC_CalibrateMT2AmpOffset();

    OpenLoopFOC_SetTarget(OPEN_LOOP_FOC_DEFAULT_UQ, OPEN_LOOP_FOC_DEFAULT_SPEED_RAD_S);
    OpenLoopFOC_SetControlPeriod(OPEN_LOOP_FOC_DEFAULT_CONTROL_PERIOD);

    TIM6_Control_Init(TIM6_CONTROL_DEFAULT_PERIOD_MS);
    TIM6_Control_Start();
	
    App_PrintFOCState();

    while (1)
    {
        App_KeyTask();
        BSP_ADC_Update();

        adc_print_tick++;
        if (adc_print_tick >= APP_ADC_PRINT_PERIOD_MS)
        {
            adc_print_tick = 0u;
            App_PrintADCState();
        }
    }
}
