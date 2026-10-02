#include "remote.h"

extern "C" uint8_t remote_rx_buf[18];

Remote remote(&huart3);

constexpr uint16_t REMOTE_CONNECT_TIMEOUT = 500u;
// Constructor 构造函数
Remote::Remote(UART_HandleTypeDef *huart): huart_(huart), connect_(REMOTE_CONNECT_TIMEOUT){
    switch_.l = RCSwitchState_e::DOWN;
    switch_.r = RCSwitchState_e::DOWN;
}

// Start UART(SBUS) receive. 打开UART接收
void Remote::init() {
    // Your code here
    if (HAL_UARTEx_ReceiveToIdle_DMA(huart_,rx_buf,18)!=HAL_OK)
    {
        Error_Handler();
    }
}

// Reset RC data. 重置遥控器数据
void Remote::reset() {
    // Your code here
    channel_.l_row = 1024;
    channel_.r_row = 1024;
    channel_.l_col = 1024;
    channel_.r_col = 1024;
    channel_.dial_wheel = 1024;
    switch_.l = RCSwitchState_e::DOWN;
    switch_.r = RCSwitchState_e::DOWN;

}

// Check for uart correspondence. 检查串口是否匹配
bool Remote::rxMsgCheck(UART_HandleTypeDef *huart) const {
    // Your code here
    if (huart == huart_){return true;}
    else{return false;}
}

// Update connect status, restart UART(SBUS) receive.
// 更新连接状态，重新打开UART(SBUS)接收
void Remote::rxMsgCallback(uint8_t *data){
    // Your code here
    for (uint16_t i = 0;i < 18;i++)
    {
        rx_data_[i] = data[i];
    }
    handle();
}

// Unpack data. 数据解包->处理数据
void Remote::handle() {
    // Your code here
    channel_.r_col = (rx_data_[0] | (rx_data_[1] << 8)) & 0x7FF;
    channel_.r_row = (rx_data_[1]>>3 | (rx_data_[2] << 5)) & 0x7FF;
    channel_.l_col = (rx_data_[2]>>6 | (rx_data_[3] << 2) | (rx_data_[4] << 10)) & 0x7FF;
    channel_.l_row = (rx_data_[4] >> 1 | (rx_data_[5] << 7)) & 0x7FF;
    switch_.r = static_cast<RCSwitchState_e> (rx_data_[5]>>4 & 0x03);
    switch_.l = static_cast<RCSwitchState_e> (rx_data_[5]>>6 & 0x03);
    channel_.dial_wheel = (rx_data_[16] | (rx_data_[17] << 8)) & 0x7FF;
}
extern "C" void Remote_Process(uint8_t *data)
{
    remote.rxMsgCallback(data);
}

extern "C" {
    void Remote_Init(void){Remote_Init();}
    void Remote_handle(void){Remote_handle();}
}