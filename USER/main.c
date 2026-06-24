#include "stm32f4xx.h"
#include "usart.h"
#include "bsp_systick.h"
#include "bsp_key.h"
#include "bsp_tim8_pwm.h"
#include "bsp_tim6_control.h"
#include "bsp_adc.h"
#include "bsp_vofa.h"
#include "foc_math.h"
#include "foc_close_loop.h"

#define APP_IQ_DEFAULT            0.30f
#define APP_IQ_STEP               0.05f
#define APP_IQ_MIN               -2.00f
#define APP_IQ_MAX                2.00f
#define APP_SPEED_DEFAULT         6.2831853f
#define APP_SPEED_STEP_RAD_S      6.2831853f
#define APP_CLOSE_LOOP_KP         0.05f
#define APP_CLOSE_LOOP_KI         5.00f
#define APP_CLOSE_LOOP_LIMIT      1.00f
#define APP_PRINT_PERIOD_MS       10u

static float app_iq_ref = APP_IQ_DEFAULT;
static float app_speed_ref = APP_SPEED_DEFAULT;

static void App_PrintFOCState(void)
{
    FOC_CloseLoopStateTypeDef state;

    state = FOC_CloseLoop_GetState();

VOFA_SendJustFloat(state.IdFeedback,
                   state.IqFeedback,
                   state.UdOutput);
}

static void App_KeyTask(void)
{
    if (KEY1_EdgeRead() == KEY_DOWN)
    {
        if (FOC_CloseLoop_GetIsRunning() != 0u)
        {
            FOC_CloseLoop_Stop();
        }
        else
        {
            FOC_CloseLoop_SetTarget(0.0f, app_iq_ref);
            FOC_CloseLoop_SetSpeed(app_speed_ref);
            FOC_CloseLoop_Start();
        }

        App_PrintFOCState();
    }

    if (KEY2_EdgeRead() == KEY_DOWN)
    {
        app_iq_ref += APP_IQ_STEP;
        app_iq_ref = FOC_Clamp(app_iq_ref, APP_IQ_MIN, APP_IQ_MAX);
        FOC_CloseLoop_SetTarget(0.0f, app_iq_ref);
        App_PrintFOCState();
    }

    if (KEY3_EdgeRead() == KEY_DOWN)
    {
        app_iq_ref -= APP_IQ_STEP;
        app_iq_ref = FOC_Clamp(app_iq_ref, APP_IQ_MIN, APP_IQ_MAX);
        FOC_CloseLoop_SetTarget(0.0f, app_iq_ref);
        App_PrintFOCState();
    }

    if (KEY4_EdgeRead() == KEY_DOWN)
    {
        app_speed_ref += APP_SPEED_STEP_RAD_S;
        FOC_CloseLoop_SetSpeed(app_speed_ref);
        App_PrintFOCState();
    }

    if (KEY5_EdgeRead() == KEY_DOWN)
    {
        app_speed_ref -= APP_SPEED_STEP_RAD_S;
        FOC_CloseLoop_SetSpeed(app_speed_ref);
        App_PrintFOCState();
    }
}

int main(void)
{
    uint16_t print_tick = 0u;

    SysTick_Init();
    uart_init(115200);

    KEY_GPIO_Init();
    KEY_EdgeStateReset();

    TIM8_PWM_ModuleInit();
    BSP_ADC_Init();

    FOC_CloseLoop_Init();
    FOC_CloseLoop_Stop();

    BSP_DelayMs(100);
    BSP_ADC_CalibrateMT2AmpOffset();

    FOC_CloseLoop_SetTarget(0.0f, app_iq_ref);
    FOC_CloseLoop_SetSpeed(app_speed_ref);
    FOC_CloseLoop_SetBusVoltage(FOC_CLOSE_LOOP_DEFAULT_BUS_VOLTAGE);
    FOC_CloseLoop_SetControlPeriod(FOC_CLOSE_LOOP_DEFAULT_CONTROL_PERIOD);
    FOC_CloseLoop_SetPI(APP_CLOSE_LOOP_KP, APP_CLOSE_LOOP_KI);
    FOC_CloseLoop_SetOutputLimit(APP_CLOSE_LOOP_LIMIT);

    TIM6_Control_Init(TIM6_CONTROL_DEFAULT_PERIOD_MS);
    TIM6_Control_Start();

    App_PrintFOCState();

    while (1)
    {
        App_KeyTask();

        print_tick++;
        if (print_tick >= APP_PRINT_PERIOD_MS)
        {
            print_tick = 0u;
            App_PrintFOCState();
        }
    }
}