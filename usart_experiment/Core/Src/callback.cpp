//
// Created by qingx on 2026/10/2.
//
#include "usart.h"
#include "main.h"
extern uint8_t rx_msg[4];

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        if (rx_msg[0] == 'R') HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
        else if (rx_msg[0] == 'M') HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
        HAL_UART_Receive_DMA(&huart1, rx_msg, 10);
    }
}