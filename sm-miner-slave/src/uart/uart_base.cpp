#include "uart_base.hpp"

UartBase::UartBase(const UartConfig &_uc)
    : uc(_uc),
      inTransmit(false),
      dma_rx_idx_(0),
      dma_tx_buff_len(0),
      dma_tx_buff_ptr(nullptr),
      dma_rx_buff_len(0),
      dma_rx_buff_ptr(nullptr),
      rxMode(RX_MODE_DMA_POLL),
      userRxMgr(nullptr),
      txDelay(0),
      lastTxTime(0),
      lastRxTime(0)
{}

void UartBase::init(uint32_t speed)
{
    uc.configureHw();

    USART_InitTypeDef USART_InitStructure;
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = speed;

    if(speed > 500000) // Enable owersampling for hige speed.
        USART_OverSampling8Cmd(uc.usart, ENABLE);
    else
        USART_OverSampling8Cmd(uc.usart, DISABLE);

    USART_Init(uc.usart, &USART_InitStructure);
    USART_Cmd (uc.usart, ENABLE);

    init_dma_tx_buff(256);
    init_dma_rx_buff(256);

    reinit_rx_dma();
}

void UartBase::init_dma_tx_buff(uint16_t len)
{
    if (!dma_tx_buff_ptr)
    {
        dma_tx_buff_len = len;
        dma_tx_buff_ptr = g_staticAllocator.alloc(dma_tx_buff_len);
    }
}

void UartBase::init_dma_rx_buff(uint16_t len)
{
    if (!dma_rx_buff_ptr)
    {
        dma_rx_buff_len = len;
        dma_rx_buff_ptr = g_staticAllocator.alloc(dma_rx_buff_len);
    }
}

void UartBase::init_tx_rb(uint8_t *ptr, uint16_t len)
{
    tx_rb_.setBuffer(ptr, len);
}

void UartBase::setTxDelay(uint32_t t) { txDelay = t; }

void UartBase::setRxMgr(UartRxMgr *_userRxMgr)
{
    if (_userRxMgr) {
        rxMode = RX_MODE_DMA_USER_RX_MGR;
        userRxMgr = _userRxMgr;
    }
}

void UartBase::onTxStart()
{
    lastTxTime = getMiliSeconds();
}

void UartBase::onTxComplete()
{}

void UartBase::send_byte(uint8_t b)
{
    USART_SendData(uc.usart, b);
}

void UartBase::send_poll_data(const uint8_t *data, uint8_t size)
{
    onTxStart();
    for(uint8_t idx = 0; idx < size; ++idx)
    {
        /* Loop until USARTy DR register is empty */
        while(USART_GetFlagStatus(uc.usart, USART_FLAG_TXE) == RESET){}
        send_byte(data[idx]);
    }
    while(USART_GetFlagStatus(uc.usart, USART_FLAG_TXE) == RESET){}
    onTxComplete();
}

void UartBase::send_poll_data(const char *data, uint8_t size)
{
    send_poll_data(reinterpret_cast<uint8_t*>(const_cast<char*>(data)), size);
}

uint16_t UartBase::send_buff_dma(const uint8_t *buf, uint16_t size)
{
    if(inTransmit)
        return 0;
    if (size > dma_tx_buff_len)
        return 0;
    memcpy(dma_tx_buff_ptr, buf, size);
    reinit_tx_dma(size);
    return size;
}

void UartBase::dma_TX_IRQ_handler()
{
    if ( DMA_GetITStatus(uc.txDmaStream, uc.txDmaIntTC) )//Streamx transfer complete interrupt
    {
        DMA_ClearITPendingBit(uc.txDmaStream, uc.txDmaIntTC);
        DMA_ITConfig         (uc.txDmaStream, DMA_IT_TC, DISABLE);

        if (txDelay > 0) {
            g_timerMgr.registerHandler(TID_TIM2, this);
            g_timerMgr.start(TID_TIM2, txDelay);
        }
        else
        {
            dma_TX_Complete();
        }
    }
}

void UartBase::onTimer(TimerId tid)
{
    if (tid == TID_TIM2)
    {
        dma_TX_Delay_Complete();
    }
}

void UartBase::dma_TX_Delay_Complete()
{
    dma_TX_Complete();
}

void UartBase::dma_TX_Complete()
{
    if(fill_tx_dma_from_rb() == 0)
    {
        onTxComplete();
        inTransmit = false;
    }
}

uint32_t UartBase::write(const uint8_t *dat, uint32_t size)
{
    uint32_t res = 0;
    {
        AtomicBlock ab;
        while( (!tx_rb_.is_full()) && (res < size) )
        {
            tx_rb_.push_back(dat[res++]);
        }
    }
    {
        AtomicBlock ab;
        if(inTransmit == false)
        {
            fill_tx_dma_from_rb();
        }
    }
    return res;
}

uint16_t UartBase::fill_tx_dma_from_rb()
{
    uint16_t data_size;
    data_size = (tx_rb_.size() > dma_tx_buff_len) ? dma_tx_buff_len : tx_rb_.size();
    for(uint16_t idx = 0; idx < data_size; ++idx)
        dma_tx_buff_ptr[idx] = tx_rb_.pop_back();

    if(data_size)
        reinit_tx_dma(data_size);

    return data_size;
}

void UartBase::dma_RX_IRQ_handler()
{
    const uint32_t halfSize = dma_rx_buff_len / 2;

    if (DMA_GetITStatus(uc.rxDmaStream, uc.rxDmaIntHT))
    {
        if (rxMode == RX_MODE_DMA_USER_RX_MGR && userRxMgr)
        {
            lastRxTime = getMiliSeconds();
            userRxMgr->onRxSegment(dma_rx_buff_ptr, halfSize);
        }
        DMA_ClearITPendingBit(uc.rxDmaStream, uc.rxDmaIntHT);
    }

    if (DMA_GetITStatus(uc.rxDmaStream, uc.rxDmaIntTC))
    {
        if (rxMode == RX_MODE_DMA_USER_RX_MGR && userRxMgr)
        {
            lastRxTime = getMiliSeconds();
            userRxMgr->onRxSegment(dma_rx_buff_ptr + halfSize, halfSize);
        }
        DMA_ClearITPendingBit(uc.rxDmaStream, uc.rxDmaIntTC);
    }
}

void UartBase::receive_rx_dma()
{
    uint16_t dma_idx = dma_rx_buff_len - rx_dma_index();
    while( dma_idx !=  dma_rx_idx_)
    {
        rx_rb_.push_back(dma_rx_buff_ptr[dma_rx_idx_]);
        dma_rx_idx_++;
        if(dma_rx_idx_ == dma_rx_buff_len)
            dma_rx_idx_ = 0;
    }
}

void UartBase::uart_IRQ_handler()
{
    if(USART_GetITStatus(uc.usart, USART_IT_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        USART_ReceiveData(uc.usart);
    }
}

bool UartBase::is_TX_ready() const {return !inTransmit;}

void UartBase::reinit_tx_dma(uint32_t data_size)
{
    onTxStart();
    inTransmit = true;
    uc.enableDmaClock();
    DMA_DeInit(uc.txDmaStream);
    DMA_InitTypeDef dma_init;

    dma_init.DMA_Channel            = uc.txDmaChannel;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)uc.usart + 0x04;
    dma_init.DMA_Memory0BaseAddr    = (uint32_t)(dma_tx_buff_ptr);
    dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
    dma_init.DMA_BufferSize         = data_size;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Normal;
    dma_init.DMA_Priority           = DMA_Priority_Medium;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Enable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_3QuartersFull;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init(uc.txDmaStream, &dma_init);
    DMA_Cmd (uc.txDmaStream, ENABLE);

    USART_DMACmd(uc.usart, USART_DMAReq_Tx, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = uc.txDmaStreamIRQ;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_ClearFlag(uc.usart, USART_FLAG_TC);

    DMA_ITConfig(uc.txDmaStream, DMA_IT_TC , ENABLE);
}

void UartBase::reinit_rx_dma()
{
    uc.enableDmaClock();

    DMA_DeInit(uc.rxDmaStream);

    DMA_InitTypeDef dma_init;
    dma_init.DMA_Channel            = uc.rxDmaChannel;
    dma_init.DMA_PeripheralBaseAddr = (uint32_t)uc.usart + 0x04;
    dma_init.DMA_Memory0BaseAddr    = (uint32_t)(dma_rx_buff_ptr);
    dma_init.DMA_DIR                = DMA_DIR_PeripheralToMemory;
    dma_init.DMA_BufferSize         = dma_rx_buff_len;
    dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
    dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
    dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte;
    dma_init.DMA_Mode               = DMA_Mode_Circular;
    dma_init.DMA_Priority           = DMA_Priority_Medium;
    dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
    dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_3QuartersFull;
    dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
    dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
    DMA_Init(uc.rxDmaStream, &dma_init);

    DMA_Cmd (uc.rxDmaStream, ENABLE);

    USART_DMACmd(uc.usart, USART_DMAReq_Rx, ENABLE);

    if (rxMode == RX_MODE_DMA_USER_RX_MGR)
    {
        // Enable the USART Interrupt
        NVIC_InitTypeDef nvic;
        nvic.NVIC_IRQChannel = uc.rxDmaStreamIRQ;
        nvic.NVIC_IRQChannelSubPriority = 0;
        nvic.NVIC_IRQChannelPreemptionPriority = 0;
        nvic.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&nvic);

        DMA_ITConfig(uc.rxDmaStream, DMA_IT_HT | DMA_IT_TC , ENABLE);
    }
}
