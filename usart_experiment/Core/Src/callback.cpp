#include "main.h"
#include "usart.h"
#include "remote.h"
extern uint8_t remote_rx_buf[18];
extern "C" void Remote_Process(uint8_t *data);
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart,uint16_t Size)
{
    if (huart->Instance == USART3)
    {
        if (Size ==18 )
        {
            Remote_Process(remote_rx_buf);
            //重新启动DMA
            if (HAL_UARTEx_ReceiveToIdle_IT(&huart3,remote_rx_buf,18) != HAL_OK)
            {
                remote.init();
            }
        }
    }
}