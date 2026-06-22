/* -----------------------按键驱动 -------------------*/
#include "bsp_key.h"

// 初始化GPIO: PE0~PE4配置为浮空输入
void KEY_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(KEY1_RCC_CLOCKGPIO|KEY2_RCC_CLOCKGPIO|KEY3_RCC_CLOCKGPIO|KEY4_RCC_CLOCKGPIO|KEY5_RCC_CLOCKGPIO, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

    GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;
    GPIO_Init(KEY1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN;
    GPIO_Init(KEY2_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY3_GPIO_PIN;
    GPIO_Init(KEY3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY4_GPIO_PIN;
    GPIO_Init(KEY4_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = KEY5_GPIO_PIN;
    GPIO_Init(KEY5_GPIO, &GPIO_InitStructure);
}

/* -----------------------阻塞型按键读取 -------------------*/
// 软件消抖延时
static void KEY_ScanDelay(void)
{
    for (volatile uint32_t i = 0; i < 1000; i++)
        for (volatile uint32_t j = 0; j < 100; j++);
}

// 通用阻塞型读取: 消抖+等待松手后返回
static KEYState_TypeDef KEY_StateRead(GPIO_TypeDef *gpio, uint16_t pin, uint8_t down_level)
{
    if (GPIO_ReadInputDataBit(gpio, pin) == (BitAction)down_level)
    {
        KEY_ScanDelay();
        if (GPIO_ReadInputDataBit(gpio, pin) == (BitAction)down_level)
        {
            while (GPIO_ReadInputDataBit(gpio, pin) == (BitAction)down_level);
            return KEY_DOWN;
        }
    }
    return KEY_UP;
}

KEYState_TypeDef KEY1_StateRead(void) { return KEY_StateRead(KEY1_GPIO, KEY1_GPIO_PIN, KEY1_DOWN_LEVEL); }
KEYState_TypeDef KEY2_StateRead(void) { return KEY_StateRead(KEY2_GPIO, KEY2_GPIO_PIN, KEY2_DOWN_LEVEL); }
KEYState_TypeDef KEY3_StateRead(void) { return KEY_StateRead(KEY3_GPIO, KEY3_GPIO_PIN, KEY3_DOWN_LEVEL); }
KEYState_TypeDef KEY4_StateRead(void) { return KEY_StateRead(KEY4_GPIO, KEY4_GPIO_PIN, KEY4_DOWN_LEVEL); }
KEYState_TypeDef KEY5_StateRead(void) { return KEY_StateRead(KEY5_GPIO, KEY5_GPIO_PIN, KEY5_DOWN_LEVEL); }

/* -----------------------非阻塞型边沿检测 -------------------*/
// 记录每个按键的上一次状态 (0=未按下, 1=按下)
static uint8_t key1_edge_last = 0u;
static uint8_t key2_edge_last = 0u;
static uint8_t key3_edge_last = 0u;
static uint8_t key4_edge_last = 0u;
static uint8_t key5_edge_last = 0u;

// 通用边沿检测: 按下瞬间(仅一次)返回KEY_DOWN
static KEYState_TypeDef KEY_EdgeRead(GPIO_TypeDef *gpio, uint16_t pin, uint8_t down_level, uint8_t *last_state)
{
    uint8_t current_state = (GPIO_ReadInputDataBit(gpio, pin) == (BitAction)down_level);

    // 边沿: 本次=按下 且 上次=未按下
    if (current_state == 1u && *last_state == 0u)
    {
        *last_state = current_state;
        return KEY_DOWN;
    }
    *last_state = current_state;
    return KEY_UP;
}

// 重置所有按键边沿状态
void KEY_EdgeStateReset(void)
{
    key1_edge_last = 0u;
    key2_edge_last = 0u;
    key3_edge_last = 0u;
    key4_edge_last = 0u;
    key5_edge_last = 0u;
}

KEYState_TypeDef KEY1_EdgeRead(void) { return KEY_EdgeRead(KEY1_GPIO, KEY1_GPIO_PIN, KEY1_DOWN_LEVEL, &key1_edge_last); }
KEYState_TypeDef KEY2_EdgeRead(void) { return KEY_EdgeRead(KEY2_GPIO, KEY2_GPIO_PIN, KEY2_DOWN_LEVEL, &key2_edge_last); }
KEYState_TypeDef KEY3_EdgeRead(void) { return KEY_EdgeRead(KEY3_GPIO, KEY3_GPIO_PIN, KEY3_DOWN_LEVEL, &key3_edge_last); }
KEYState_TypeDef KEY4_EdgeRead(void) { return KEY_EdgeRead(KEY4_GPIO, KEY4_GPIO_PIN, KEY4_DOWN_LEVEL, &key4_edge_last); }
KEYState_TypeDef KEY5_EdgeRead(void) { return KEY_EdgeRead(KEY5_GPIO, KEY5_GPIO_PIN, KEY5_DOWN_LEVEL, &key5_edge_last); }
