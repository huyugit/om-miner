#ifndef UART_BASE_HPP
#define UART_BASE_HPP

#include <stm_gpio.h>
#include <array.hpp>
#include <string.h>
#include <atomic_block.hpp>

#include "uart_config.hpp"
#include "static_allocator.h"
#include "timer.h"
#include "mytime.h"


class UartRxMgr
{
public:
    virtual void onRxSegment(uint8_t *ptr, uint32_t size) = 0;
};


class UartBase
        : public TimerEventHandler
{
public:
    enum RxMode {
        RX_MODE_DMA_POLL,
        RX_MODE_DMA_USER_RX_MGR
    };

public:
    UartConfig uc;

    UartBase(const UartConfig& _uc);

    void init(uint32_t speed = 115200);

    // Note: MUST be called before init method!
    void init_dma_tx_buff(uint16_t len);
    void init_dma_rx_buff(uint16_t len);

    void init_tx_rb(uint8_t *ptr, uint16_t len);

    void setTxDelay(uint32_t t);

    void setRxMgr(UartRxMgr *_userRxMgr);

    void onTxStart();
    void onTxComplete();

    void send_byte(uint8_t b);

    void send_poll_data(const uint8_t* data, uint8_t size);
    void send_poll_data(const char* data, uint8_t size);

    uint16_t send_buff_dma(const uint8_t* buf, uint16_t size);

    void dma_TX_IRQ_handler();

    void onTimer(TimerId tid);

    void dma_TX_Delay_Complete();
    void dma_TX_Complete();

    uint32_t write(const uint8_t* dat, uint32_t size);
    uint16_t fill_tx_dma_from_rb();

    void dma_RX_IRQ_handler();

    void receive_rx_dma();
    void    inline receive         () {receive_rx_dma();}
    int     inline rx_buf_size     () {return rx_rb_.size();}
    int     inline tx_buf_size     () {return tx_rb_.size();}
    int     inline tx_buf_free_size() {return (tx_rb_.capasity() - tx_rb_.size());}
    void    inline rx_clear        () {rx_rb_.clear();}
    uint8_t inline rx_pop_back     () {return rx_rb_.pop_back();}

    void uart_IRQ_handler();
    bool is_TX_ready()const;

private:
    void reinit_tx_dma(uint32_t data_size);
    void reinit_rx_dma();

    inline uint16_t rx_dma_index(){return uc.rxDmaStream->NDTR;}
    uint8_t *       rx_dma_buf  (){return dma_rx_buff_ptr;}

private:
    volatile bool inTransmit;
    uint16_t dma_rx_idx_;

    uint16_t dma_tx_buff_len;
    uint8_t* dma_tx_buff_ptr;

    uint16_t dma_rx_buff_len;
    uint8_t* dma_rx_buff_ptr;

    RingBufferDyn<uint8_t, 4> rx_rb_;
    RingBufferDyn<uint8_t, 4> tx_rb_;

    RxMode rxMode;
    UartRxMgr *userRxMgr;

    uint32_t txDelay;

public:
    uint32_t lastTxTime;
    uint32_t lastRxTime;

    //uint32_t linkActivityTime[id][op] = getMiliSeconds();
};

#endif // UART_BASE_HPP
