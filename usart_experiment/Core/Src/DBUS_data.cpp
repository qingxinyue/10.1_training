//
// Created by qingx on 2026/10/2.
//
#include "main.h"
#include "usart.h"
#include <cstring>

struct DBUSdata
{
    int16_t channel[4];
    uint16_t s1;
    uint16_t s2;

    uint16_t mouse_x;
    uint16_t mouse_y;
    uint16_t mouse_z;
    uint8_t mouse_left;
    uint8_t mouse_right;
    uint16_t button;
};

class Dbus
{
    static const uint16_t DATA_SIZE = 18;
    uint8_t rx_buf[36]{};
    uint8_t rx_data[DATA_SIZE]{};

    DBUSdata data{};
    volatile uint8_t tx_state = 0;
    volatile uint8_t tx_len = 0;
    volatile bool rx_fault = false;

    uint32_t last_valid_tick =0;
    bool has_valid_frame = false;

public:
    HAL_StatusTypeDef Echo_Initial()
    {
        return HAL_UARTEx_ReceiveToIdle_DMA(&huart1, rx_data,DATA_SIZE *8);
    }

    void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart ,uint16_t size)
    {
        if (tx_state == 0){
            if (huart == &huart1)
            {
                std::memcpy(rx_data,rx_buf,size);
                tx_state = 1;
            }
            if (HAL_UARTEx_ReceiveToIdle_DMA(huart, rx_data,DATA_SIZE*8) != HAL_OK)
            {
                rx_fault = true;
            }
        }
    }

    void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
    {
        if (huart == &huart1)
        {
            tx_state = 0;
        }
    }

    bool connected(uint32_t timeout_ms =1000)
    {
        uint32_t tick = HAL_GetTick();
        if (tick - last_valid_tick < timeout_ms)
        {
            last_valid_tick = tick;
            return true;
        }
        return false;
    }

    static int16_t readInt16(const uint8_t *num)
    {
        uint16_t low = num[0];
        uint16_t high = num[1];
        uint16_t rawVal =low|(high<<8);

        int32_t realVal =rawVal;
        if (realVal & 0x8000U)
        {
            realVal-=65536;
        }
        return static_cast<int16_t>(realVal);
    }

    bool handle()
    {
        const uint8_t *raw=rx_data;
        DBUSdata next{};

        uint16_t raw_chan[4];

        raw_chan[0]=(uint32_t(raw[0]) | (uint32_t(raw[1])<<8)) & 0x07FFU;
        raw_chan[1]=((uint32_t(raw[2])>>3) | (uint32_t(raw[3])<<5)) & 0x07FFU;
        raw_chan[2]=((uint32_t(raw[3])>>6) | (uint32_t(raw[4])<<2)) & 0x07FFU;
        raw_chan[3]=((uint32_t(raw[4])>>1) | (uint32_t(raw[5])<<7)) & 0x07FFU;

        for (int i=0;i<4;i++)
        {
            if (raw_chan[i] <364 || raw_chan[i] >1684) return false;
        }
        next.s1=(raw[5]>>4)& 0x03U;
        next.s2=(raw[5]>>6)& 0x03U;

        if (next.s1 <1 ||next.s2 <1 ||next.s1 >3 ||next.s2 >3) return false;

        next.mouse_x=readInt16(&raw[6]);
        next.mouse_y=readInt16(&raw[8]);
        next.mouse_z=readInt16(&raw[10]);

        next.mouse_left = raw[12];
        next.mouse_right = raw[13];

        next.button = raw[14]|(raw[15]<<8);

        last_valid_tick = HAL_GetTick();
        has_valid_frame = true;
        data=next;

        return true;
    }
};