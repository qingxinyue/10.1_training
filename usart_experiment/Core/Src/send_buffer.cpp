//
// Created by qingx on 2026/10/2.
//
#include "main.h"
#include "usart.h"
#include <cstring>

namespace
{
    constexpr uint16_t N =10;
    extern uint8_t rx_msg[N];

    uint8_t rx_msg[N];
    uint8_t tx_msg[N];

    //发送状态
    volatile uint8_t tx_state = 0;
    volatile uint8_t tx_len = 0;
    volatile bool rx_fault = false;
}

HAL_StatusTypeDef Echo_Initial()
{
    return HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_msg, N);
}

void Echo_Poll()
{
    if (tx_state != 1)
    {
        return;
    }
    tx_state = 2;
    if (HAL_UART_Transmit_IT(&huart1, tx_msg, tx_len) != HAL_OK)
    {
        tx_state = 1;
    }

}

//固定字节数转发的接收中断函数
// extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
// {
//     if (huart == &huart1)
//     {
//         if (tx_state == 0)
//         {
//             std::memcpy(tx_msg, rx_msg, N);
//             tx_state = 1;
//         }
//     }
//     if (HAL_UART_Receive_DMA(&huart1, tx_msg, N) != HAL_OK)
//     {
//         rx_fault = true;
//     }
// }

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        tx_state = 0;
    }
}
extern "C" HAL_StatusTypeDef UART_INIT(void)
{
    return Echo_Initial();
}

extern "C" void Echo_run(void)
{
    Echo_Poll();
}

//空闲中断的接收中断函数
extern "C" void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart == &huart1 &&
        (HAL_UARTEx_GetRxEventType(huart) == HAL_UART_RXEVENT_IDLE ||HAL_UARTEx_GetRxEventType(huart)== HAL_UART_RXEVENT_TC))
    {
        if (tx_state == 0 && size > 0 && size <= N)
        {
            std::memcpy(tx_msg, rx_msg, size);
            tx_len = size;
            tx_state = 1;
        }
        if (HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_msg, N) != HAL_OK)
        {
            rx_fault = true;
        }
    }
}