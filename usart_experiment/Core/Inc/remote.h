#ifndef UART_REMOTE_H
#define UART_REMOTE_H

#include "connect.hpp"
#include "usart.h"

constexpr uint16_t RC_FRAME_LEN = 18u;
constexpr uint16_t RC_RX_BUF_SIZE = 72u;

struct RemoteDiagnostics {
    volatile uint32_t rx_events = 0;
    volatile uint32_t received_bytes = 0;
    volatile uint32_t valid_frames = 0;
    volatile uint32_t invalid_frames = 0;
    volatile uint32_t partial_frames = 0;
    volatile uint32_t uart_errors = 0;
    volatile uint32_t start_failures = 0;
    volatile uint32_t recoveries = 0;
    volatile uint32_t last_uart_error = 0;
    volatile uint32_t last_start_status = HAL_OK;
};
extern RemoteDiagnostics remote_diag;

class Remote {
    UART_HandleTypeDef *huart_;
    uint8_t rx_buf[RC_RX_BUF_SIZE]{};
    uint8_t rx_data_[RC_FRAME_LEN]{};
    uint8_t assembling_[RC_FRAME_LEN]{};
    uint8_t pending_[RC_FRAME_LEN]{};
    uint16_t read_pos_ = 0;
    uint8_t assembling_len_ = 0;
    volatile bool pending_ready_ = false;
    void consume(uint16_t begin, uint16_t end);

public:
    enum class RCSwitchState_e : uint8_t { UP = 1, DOWN = 2, MID = 3 };
    Connect connect_;
    struct {
        uint16_t l_row, l_col, r_row, r_col, dial_wheel;
    } channel_{};
    struct { RCSwitchState_e l, r; } switch_{};

    explicit Remote(UART_HandleTypeDef *huart);
    bool start();
    void init();
    void reset();
    bool rxMsgCheck(UART_HandleTypeDef *huart) const;
    void onRxEvent(uint16_t size);
    void rxMsgCallback(uint8_t *data);
    void process();
    void handle();
};
extern Remote remote;

#ifdef __cplusplus
extern "C" {
#endif
void Remote_Init(void);
void Remote_Run(void);
#ifdef __cplusplus
}
#endif
#endif
