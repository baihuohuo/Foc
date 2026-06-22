#include "sys.h"
#include "usart.h"

// ==============================================================
// printf 重定向配置
// 无需选择 MicroLIB, 直接使用标准库
// ==============================================================
#pragma import(__use_no_semihosting)

// 标准库需要的文件句柄结构
struct __FILE {
    int handle;
};

FILE __stdout;

// 半主机模式避免函数
void _sys_exit(int x)
{
    x = x;
}

// 重定义 fputc, 实现 printf 输出到串口
int fputc(int ch, FILE *f)
{
    // 等待发送完成 (TC 标志位置1)
    while ((USART1->SR & 0X40) == 0);
    USART1->DR = (u8)ch;
    return ch;
}

// ==============================================================
// 串口接收缓冲区与状态
// ==============================================================
#if EN_USART1_RX
u8 USART_RX_BUF[USART_REC_LEN];  // 接收数据缓冲区
u16 USART_RX_STA = 0;            // 接收状态标志
#endif

// ==============================================================
// 串口初始化
// 使用 USART1, 引脚: TX=PB6, RX=PB7
// ==============================================================
void uart_init(u32 bound)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    // 使能 GPIO 和 USART1 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // 配置引脚复用功能
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);  // TX
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);  // RX

    // 配置 GPIO: TX为推挽输出, RX为输入
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 USART1 参数
    USART_InitStructure.USART_BaudRate            = bound;              // 波特率
    USART_InitStructure.USART_WordLength          = USART_WordLength_8b; // 8位数据
    USART_InitStructure.USART_StopBits            = USART_StopBits_1;    // 1位停止位
    USART_InitStructure.USART_Parity              = USART_Parity_No;      // 无校验
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无流控
    USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;   // 收发模式
    USART_Init(USART1, &USART_InitStructure);

    // 使能串口
    USART_Cmd(USART1, ENABLE);

    // 配置接收中断
#if EN_USART1_RX
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // NVIC 中断优先级配置
    NVIC_InitStructure.NVIC_IRQChannel                   = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;  // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 3;   // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd               = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}

// ==============================================================
// USART1 中断服务程序
// 接收以 0x0D 0x0A (回车换行) 结尾的数据
// ==============================================================
#if EN_USART1_RX
void USART1_IRQHandler(void)
{
    u8 Res;

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        // 读取接收到的数据
        Res = USART_ReceiveData(USART1);

        if ((USART_RX_STA & 0x8000) == 0) {
            // 还未接收完成
            if (USART_RX_STA & 0x4000) {
                // 上一字节是 0x0D, 检查是否 0x0A
                if (Res != 0x0A) {
                    // 接收错误, 重新开始
                    USART_RX_STA = 0;
                } else {
                    // 接收完成
                    USART_RX_STA |= 0x8000;
                }
            } else {
                // 还未收到 0x0D
                if (Res == 0x0D) {
                    // 标记已收到回车符
                    USART_RX_STA |= 0x4000;
                } else {
                    // 正常数据, 存入缓冲区
                    USART_RX_BUF[USART_RX_STA & 0X3FFF] = Res;
                    USART_RX_STA++;

                    // 防止缓冲区溢出
                    if (USART_RX_STA > (USART_REC_LEN - 1)) {
                        USART_RX_STA = 0;
                    }
                }
            }
        }
    }
}
#endif
