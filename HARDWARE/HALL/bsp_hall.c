#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "bsp_hall.h"

static BSP_HALL_DataTypeDef bsp_hall_data;

static void BSP_HALL_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = BSP_HALL_U_GPIO_PIN | BSP_HALL_V_GPIO_PIN | BSP_HALL_W_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOH, &GPIO_InitStructure);
}

//角度归一化
static float BSP_HALL_NormalizeAngle(float angle_rad)
{
    while (angle_rad >= BSP_HALL_TWO_PI)
    {
        angle_rad -= BSP_HALL_TWO_PI;
    }

    while (angle_rad < 0.0f)
    {
        angle_rad += BSP_HALL_TWO_PI;
    }

    return angle_rad;
}

static BSP_HALL_DirectionTypeDef BSP_HALL_GetDirectionByState(uint8_t last_state, uint8_t now_state)
{
    if (last_state == now_state)
    {
        return BSP_HALL_DIR_UNKNOWN;
    }

    /* 默认正转 Hall 顺序: 001 -> 101 -> 100 -> 110 -> 010 -> 011 */
    if (((last_state == 0x1u) && (now_state == 0x5u)) ||
        ((last_state == 0x5u) && (now_state == 0x4u)) ||
        ((last_state == 0x4u) && (now_state == 0x6u)) ||
        ((last_state == 0x6u) && (now_state == 0x2u)) ||
        ((last_state == 0x2u) && (now_state == 0x3u)) ||
        ((last_state == 0x3u) && (now_state == 0x1u)))
    {
        return BSP_HALL_DIR_FORWARD;
    }

    /* 反转 Hall 顺序 */
    if (((last_state == 0x1u) && (now_state == 0x3u)) ||
        ((last_state == 0x3u) && (now_state == 0x2u)) ||
        ((last_state == 0x2u) && (now_state == 0x6u)) ||
        ((last_state == 0x6u) && (now_state == 0x4u)) ||
        ((last_state == 0x4u) && (now_state == 0x5u)) ||
        ((last_state == 0x5u) && (now_state == 0x1u)))
    {
        return BSP_HALL_DIR_REVERSE;
    }

    return BSP_HALL_DIR_UNKNOWN;
}

void BSP_HALL_Init(void)
{
	BSP_HALL_GPIO_Init();

	bsp_hall_data.HallU = 0u;
    bsp_hall_data.HallV = 0u;
    bsp_hall_data.HallW = 0u;
    bsp_hall_data.HallState = BSP_HALL_INVALID_STATE;
    bsp_hall_data.LastHallState = BSP_HALL_INVALID_STATE;

    bsp_hall_data.RawElectricalAngle = 0.0f;
    bsp_hall_data.ElectricalAngle = 0.0f;
    bsp_hall_data.AngleOffset = BSP_HALL_DEFAULT_ANGLE_OFFSET;

    bsp_hall_data.ElectricalSpeedRadS = 0.0f;
    bsp_hall_data.TimeFromLastEdgeSec = 0.0f;
    bsp_hall_data.LastEdgePeriodSec = 0.0f;

    bsp_hall_data.Direction = BSP_HALL_DIR_UNKNOWN;
}

//dt_sec: 两次调用 BSP_HALL_Update() 的时间间隔, 单位: s
void BSP_HALL_Update(float dt_sec)
{
    uint8_t now_state;
    float interp_time;
    float continuous_angle;

    if(dt_sec < 0) dt_sec = 0.0f;

     //累计距离上一次 Hall 跳变过去了多久
    bsp_hall_data.TimeFromLastEdgeSec += dt_sec;

    now_state = BSP_HALL_ReadState();
    if(BSP_HALL_IsValidState(now_state) == 0u)
    {
        bsp_hall_data.HallState = now_state;
        bsp_hall_data.Direction = BSP_HALL_DIR_UNKNOWN;
        return ;
    }

    bsp_hall_data.HallU = (uint8_t)((now_state >> 2) & 0x01u);
    bsp_hall_data.HallV = (uint8_t)((now_state >> 1) & 0x01u);
    bsp_hall_data.HallW = (uint8_t)(now_state & 0x01u);

    //霍尔状态发生跳变
	if (now_state != bsp_hall_data.LastHallState)
	{
		if (BSP_HALL_IsValidState(bsp_hall_data.LastHallState) == 1u)
		{
			bsp_hall_data.Direction = BSP_HALL_GetDirectionByState(bsp_hall_data.LastHallState, now_state);

            //电角速度计算
            if (bsp_hall_data.TimeFromLastEdgeSec > 0.000001f)//防止电机停转或低速卡住时，ElectricalAngle 会继续漂 
            {
                bsp_hall_data.LastEdgePeriodSec = bsp_hall_data.TimeFromLastEdgeSec;

                if (bsp_hall_data.Direction == BSP_HALL_DIR_FORWARD)
                {
                    bsp_hall_data.ElectricalSpeedRadS = BSP_HALL_STEP_ANGLE_RAD /
                                                        bsp_hall_data.LastEdgePeriodSec;
                }
                else if (bsp_hall_data.Direction == BSP_HALL_DIR_REVERSE)
                {
                    bsp_hall_data.ElectricalSpeedRadS = -BSP_HALL_STEP_ANGLE_RAD /
                                                         bsp_hall_data.LastEdgePeriodSec;
                }
                else
                {
                    bsp_hall_data.ElectricalSpeedRadS = 0.0f;
                }
            }
		}
		else
		{
            // 第一次进入时, 不能判断方向和速度
			bsp_hall_data.Direction = BSP_HALL_DIR_UNKNOWN;
            bsp_hall_data.ElectricalSpeedRadS = 0.0f;
		}

		bsp_hall_data.LastHallState = now_state;
        bsp_hall_data.TimeFromLastEdgeSec = 0.0f;
        bsp_hall_data.RawElectricalAngle = BSP_HALL_StateToElectricalAngle(now_state);
	}

	bsp_hall_data.HallState = now_state;

    //Hall 状态没变时, 它会一直增长,用它乘以速度, 就能得到两个状态之间的连续角度变化
    interp_time = bsp_hall_data.TimeFromLastEdgeSec;

    //限幅:如果电机已经停了, 不能让角度一直按照旧速度继续增长
    if ((bsp_hall_data.LastEdgePeriodSec > 0.0f) &&
    (interp_time > bsp_hall_data.LastEdgePeriodSec))
    {
        interp_time = bsp_hall_data.LastEdgePeriodSec;
    }

    //连续角度 = Hall 离散角度 + 速度 * 距离上次跳变时间 + 偏置角
    continuous_angle = bsp_hall_data.RawElectricalAngle +
                       bsp_hall_data.ElectricalSpeedRadS * interp_time +
                       bsp_hall_data.AngleOffset;

    bsp_hall_data.ElectricalAngle = BSP_HALL_NormalizeAngle(continuous_angle);
}

uint8_t BSP_HALL_ReadState(void)
{
	uint8_t hall_u;
	uint8_t hall_v;
	uint8_t hall_w;
    uint8_t hall_state;
	
	hall_u = GPIO_ReadInputDataBit(BSP_HALL_U_GPIO_PORT, BSP_HALL_U_GPIO_PIN);
    hall_v = GPIO_ReadInputDataBit(BSP_HALL_V_GPIO_PORT, BSP_HALL_V_GPIO_PIN);
    hall_w = GPIO_ReadInputDataBit(BSP_HALL_W_GPIO_PORT, BSP_HALL_W_GPIO_PIN);
    hall_state = (((hall_u << 2) | (hall_v << 1) | hall_w) & BSP_HALL_STATE_MASK);

    return hall_state;
}


uint8_t BSP_HALL_IsValidState(uint8_t hall_state)
{
    hall_state &= BSP_HALL_STATE_MASK;

    if ((hall_state == 0x1u) ||
        (hall_state == 0x2u) ||
        (hall_state == 0x3u) ||
        (hall_state == 0x4u) ||
        (hall_state == 0x5u) ||
        (hall_state == 0x6u))
    {
        return 1u;
    }

    return 0u;
}

float BSP_HALL_StateToElectricalAngle(uint8_t hall_state)
{
    switch (hall_state & BSP_HALL_STATE_MASK)
    {
        case 0x1u:
            return 0.5235988f;   // 30 deg

        case 0x5u:
            return 1.5707963f;   // 90 deg

        case 0x4u:
            return 2.6179939f;   // 150 deg

        case 0x6u:
            return 3.6651914f;   // 210 deg

        case 0x2u:
            return 4.7123890f;   // 270 deg

        case 0x3u:
            return 5.7595865f;   // 330 deg

        default:
            return bsp_hall_data.RawElectricalAngle;
    }
}

void BSP_HALL_SetAngleOffset(float angle_offset)
{
    bsp_hall_data.AngleOffset = angle_offset;
}

float BSP_HALL_GetRawElectricalAngle(void)
{
    return bsp_hall_data.RawElectricalAngle;
}

float BSP_HALL_GetElectricalAngle(void)
{
    return bsp_hall_data.ElectricalAngle;
}

float BSP_HALL_GetElectricalSpeedRadS(void)
{
    return bsp_hall_data.ElectricalSpeedRadS;
}

BSP_HALL_DirectionTypeDef BSP_HALL_GetDirection(void)
{
	return bsp_hall_data.Direction;
}

BSP_HALL_DataTypeDef BSP_HALL_GetAllData(void)
{
    return bsp_hall_data;
}