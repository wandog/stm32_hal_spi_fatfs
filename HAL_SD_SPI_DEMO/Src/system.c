/**
 *     filename: system.h
 *
 *   Create by AnKun on 2019/12/13.
 */

#include "system.h"

uint8_t DEBUG_UART_RX_BUF[DEBUG_UART_RX_LEN];       // 串口接收缓存
uint8_t DEBUG_UART_RX_BYTE = 0;                     // 串口中断接收一个字节
uint16_t DEBUG_UART_RX_STA = 0;                     // 串口接收状态，第15位表示接收到了换行符，第14为表示接收到了回车符，第0~7位表示本次接收到的数据长度

/**
 * 调试串口初始化
 */
void DEBUG_UART_Init(void)
{
    // 初始化串口
    DEBUG_UART_INITFUN();

    // 使能串口接收中断
    HAL_UART_Receive_IT(&DEBUG_UART_HANDLER, &DEBUG_UART_RX_BYTE, 1);
}

/**
 * 重定向printf函数
 */
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&DEBUG_UART_HANDLER, (uint8_t *) &ch, 1, 10);
    return ch;
}

/**
 * 调试串口格式化输出，使用方法与printf相同
 * @param format 要格式化输出的字符串
 * @param ...    格式化参数
 */
void debug_printf(const void* format, ...)
{
    static char fmt[1024];
    va_list ap;
    va_start(ap, format);
    vsprintf(fmt, (char *)format, ap);
    va_end(ap);
    HAL_UART_Transmit(&DEBUG_UART_HANDLER, (uint8_t *)fmt, strlen(fmt), 1000);
}

/**
 * 串口接收中断回调函数
 * @param huart 串口句柄
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == DEBUG_UART_HANDLER.Instance)
    {
        if ((DEBUG_UART_RX_STA & 0x8000) == 0)                                                  // 接收未完毕
        {
            if (DEBUG_UART_RX_STA & 0x4000)                                                     // 已经接收到了回车符
            {
                if (DEBUG_UART_RX_BYTE == '\n')     DEBUG_UART_RX_STA |= 0x8000;                // 本次接收到的是换行符，标记接收到了换行符
                else                                DEBUG_UART_RX_STA = 0;                      // 本次接收到的不是换行符，视为接收错误，状态置为初始
            }
            else
            {
                if (DEBUG_UART_RX_BYTE == '\r')     DEBUG_UART_RX_STA |= 0x4000;                // 本次接收到的是回车符，标记接收到了回车符
                else
                {
                    DEBUG_UART_RX_BUF[DEBUG_UART_RX_STA & 0x3FFF] = DEBUG_UART_RX_BYTE;         // 将本次接收到的数据保存到接收缓存中
                    if (DEBUG_UART_RX_STA++ >= DEBUG_UART_RX_LEN)   DEBUG_UART_RX_STA = 0;      // 连续接收到的数据长度高于最大接收长度，则视为接收错误
                }
            }
        }
        HAL_UART_Receive_IT(huart, &DEBUG_UART_RX_BYTE, 1);                                     // 再次使能接收中断
    }
}

/**
 * 串口1接收处理函数,根据实际需求进行修改
 */
void DEBUG_UART_RecvHandler(void)
{
    if (DEBUG_UART_RX_STA & 0x8000)                                 // 有数据收到
    {
        DEBUG_UART_RX_BUF[DEBUG_UART_RX_STA & 0x3FFF] = '\0';       // 添加结束符
        printf("%s\r\n", DEBUG_UART_RX_BUF);                        // 串口打印出来
        DEBUG_UART_RX_STA = 0;                                      // 状态置为初始
    }
}
