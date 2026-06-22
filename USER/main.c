/* -----------------------搴旂敤灞? -------------------*/
#include "stm32f4xx.h"
#include "usart.h"
#include "bsp_systick.h"
#include "bsp_key.h"
#include "bsp_tim8_pwm.h"
#include "bsp_tim6_control.h"
#include "bsp_adc.h"
#include "foc_math.h"
#include "open_loop_foc.h"
#include "foc_pi_controller.h"
#include "foc_current_loop.h"
#include "bsp_vofa.h"

#define APP_UQ_STEP              0.2f
#define APP_SPEED_STEP_RAD_S     6.2831853f
#define APP_ADC_PRINT_PERIOD_MS  200u

static FOC_AlphaBetaTypeDef app_current_alpha_beta;
static FOC_DQTypeDef app_current_dq;
static FOC_DQTypeDef app_current_loop_voltage_dq;

static void App_UpdateCurrentLoop(void)
{
    BSP_ADC_DataTypeDef adc_data;
    OpenLoopFOC_StateTypeDef foc_state;
    FOC_IabcTypeDef current_abc;

    adc_data = BSP_ADC_GetAllData();
    foc_state = OpenLoopFOC_GetState();

    current_abc.Ia = adc_data.RealPhaseMT2_AMPU;
    current_abc.Ib = adc_data.RealPhaseMT2_AMPV;
    current_abc.Ic = adc_data.RealPhaseMT2_AMPW;

    app_current_alpha_beta = FOC_Clarke(current_abc);
    app_current_dq = FOC_Park(app_current_alpha_beta, foc_state.ThetaE);
    app_current_loop_voltage_dq = FOC_CurrentLoop_Update(app_current_dq);
}

static void App_PrintFOCState(void)
{
    OpenLoopFOC_StateTypeDef state;

    state = OpenLoopFOC_GetState();

//    printf("Run=%u, Uq=%.2f V, Speed=%.2f rad/s, Theta=%.2f rad\r\n",
//           (unsigned int)state.IsRunning,
//           state.UqRef,
//           state.SpeedRefRadS,
//           state.ThetaE);
}

static void App_PrintADCState(void)
{
    BSP_ADC_DataTypeDef adc_data;
    FOC_CurrentLoopStateTypeDef current_loop_state;

    adc_data = BSP_ADC_GetAllData();
    current_loop_state = FOC_CurrentLoop_GetState();

//    printf("ADC value: IU=%.2fA, IV=%.2fA, IW=%.2fA, VBUS=%.2fV, TEMP_PIN=%.2fV, IMAX=%.2fA\r\n",
//           adc_data.RealPhaseMT2_AMPU,
//           adc_data.RealPhaseMT2_AMPV,
//           adc_data.RealPhaseMT2_AMPW,
//           adc_data.RealBMT2_VBUS,
//           adc_data.RealMT2_Vtemp,
//           adc_data.RealPhaseMT2_I_MAX);

//    printf("FOC current: Ialpha=%.2fA, Ibeta=%.2fA, Id=%.2fA, Iq=%.2fA\r\n",
//           app_current_alpha_beta.Alpha,
//           app_current_alpha_beta.Beta,
//           app_current_dq.D,
//           app_current_dq.Q);

//    printf("Current loop test: IdRef=%.2fA, IqRef=%.2fA, Ud=%.2fV, Uq=%.2fV\r\n",
//           current_loop_state.IdRef,
//           current_loop_state.IqRef,
//           app_current_loop_voltage_dq.D,
//           app_current_loop_voltage_dq.Q);

    VOFA_SendJustFloat(adc_data.RealPhaseMT2_AMPU,
                        adc_data.RealPhaseMT2_AMPV,
                        adc_data.RealPhaseMT2_AMPW);
}

static void App_PrintADCCalibrationState(void)
{
    printf("ADC current offset: U=%.3fV, V=%.3fV, W=%.3fV\r\n",
           BSP_ADC_GetMT2AmpU_OffsetVoltage(),
           BSP_ADC_GetMT2AmpV_OffsetVoltage(),
           BSP_ADC_GetMT2AmpW_OffsetVoltage());
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
    BSP_ADC_CalibrateMT2AmpOffset();
    App_PrintADCCalibrationState();
    FOC_CurrentLoop_Init();
    FOC_CurrentLoop_SetTarget(0.0f, 0.5f);
    OpenLoopFOC_SetTarget(OPEN_LOOP_FOC_DEFAULT_UQ, OPEN_LOOP_FOC_DEFAULT_SPEED_RAD_S);
    OpenLoopFOC_SetControlPeriod(0.001f);
    TIM6_Control_Init(TIM6_CONTROL_DEFAULT_PERIOD_MS);
    TIM6_Control_Start();
    App_PrintFOCState();


    while (1)
    {
        App_KeyTask();
        BSP_ADC_Update();
        OpenLoopFOC_SetBusVoltage(BSP_ADC_GetRealBMT2_VBUS_Voltage());
        App_UpdateCurrentLoop();

        adc_print_tick++;
        if (adc_print_tick >= APP_ADC_PRINT_PERIOD_MS)
        {
            adc_print_tick = 0u;
            App_PrintADCState();
        }

        BSP_DelayMs(1);
    }
}
