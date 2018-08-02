/*******************************************************************************
*  file    : spi_base.hpp
*  created : 23.05.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#ifndef SPI_BASE_HPP
#define SPI_BASE_HPP

#include <stm32f4xx_dma.h>
#include <stm32f4xx.h>
#include <array.hpp>
#include <atomic_block.hpp>
#include <format.hpp>
#include "mytime.h"

#include "spi_1_config.hpp"
#include "spi_3_config.hpp"

//#define DEBUG_SPI

enum SpiState
{
    SPI_STATE_IDLE          = 0x00,
    SPI_STATE_WAIT_NO_SS    = 0x01,
    SPI_STATE_ACTIVE        = 0x02,
    SPI_STATE_DONE          = 0x03
};

enum SpiTransferFlags
{
    SPI_TX_HALF         = 0x01,
    SPI_TX_DONE         = 0x02,
    SPI_RX_HALF         = 0x04,
    SPI_RX_DONE         = 0x08,
};

template <class config>
class spi_base
{
public:
    spi_base() {
        state = SPI_STATE_IDLE;
        transferFlags = 0;
    }

    void init()
    {
        config::init_hw();
    }

    void spi_NSS_interrupt()
    {
        if (EXTI_GetITStatus(config::nss_EXTI_Line()) != RESET)
        {
            enable();
            EXTI_ClearITPendingBit(config::nss_EXTI_Line());
        }
    }

    void enable()
    {
        if (state != SPI_STATE_WAIT_NO_SS)
            return;

        if (GPIO_ReadInputDataBit(config::nss_GPIO(), config::nss_GPIO_Pin()) == 0)
        {
            //log("Can not enable SPI - SS (slave select) signal is high - wait for interrupt\n");
            return;
        }

        state = SPI_STATE_ACTIVE;

        //log("Enabling SPI\n");
        DMA_Cmd (config::rx_dma_stream(), ENABLE);
        DMA_Cmd (config::tx_dma_stream(), ENABLE);
        SPI_Cmd (config::spi(), ENABLE);
    }

    void disable()
    {
        SPI_Cmd (config::spi(), DISABLE);
        DMA_Cmd (config::rx_dma_stream(), DISABLE);
        DMA_Cmd (config::tx_dma_stream(), DISABLE);
    }

    //uint16_t send_byte(uint16_t b)
    //{
    //    /*!< Loop while DR register in not emplty */
    //    while (SPI_I2S_GetFlagStatus(config::spi(), SPI_I2S_FLAG_TXE) == RESET);
    //
    //    /*!< Send byte through the SPI1 peripheral */
    //    config::spi()->DR = b;
    //
    //    /*!< Wait to receive a byte */
    //    while (SPI_I2S_GetFlagStatus(config::spi(), SPI_I2S_FLAG_RXNE) == RESET);
    //
    //    /*!< Return the byte read from the SPI bus */
    //    return config::spi()->DR;
    //}

    void txrx(uint8_t* rxBuf, uint8_t* txBuf, uint32_t buf_size)
    {
        disable();

        // Put first byte of the stream into DataRegister!
        // As it contains data from prev transmission.
        config::spi()->DR = txBuf[0];
        txBuf++;

        reinit_rx_dma(rxBuf, buf_size);
        reinit_tx_dma(txBuf, buf_size);

        state = SPI_STATE_WAIT_NO_SS;
        transferFlags = 0;

        enable();
    }

    void reinit_rx_dma(uint8_t* buf, uint32_t buf_size)
    {
        config::enable_dma_clk();
        DMA_DeInit(config::rx_dma_stream());
        DMA_InitTypeDef dma_init;

        dma_init.DMA_Channel            = config::rx_dma_channel();
        dma_init.DMA_PeripheralBaseAddr = (uint32_t)config::spi() + 0x0C;

        dma_init.DMA_Memory0BaseAddr    = reinterpret_cast<uint32_t>(buf);
        dma_init.DMA_BufferSize         = buf_size;
        dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word;

        dma_init.DMA_DIR                = DMA_DIR_PeripheralToMemory;
        dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
        dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
        dma_init.DMA_Mode               = DMA_Mode_Normal;
        dma_init.DMA_Priority           = DMA_Priority_Medium;
        dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
        dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_3QuartersFull;
        dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
        dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
        DMA_Init(config::rx_dma_stream(), &dma_init);

        SPI_DMACmd(config::spi(), SPI_I2S_DMAReq_Rx, ENABLE);

        DMA_ITConfig(config::rx_dma_stream(), DMA_IT_TC | DMA_IT_HT , ENABLE);
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Enable the SPI Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel                   = config::rx_dma_IRQ_channel();
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }

    void reinit_tx_dma(uint8_t* buf, uint32_t buf_size)
    {
        config::enable_dma_clk();
        DMA_DeInit(config::tx_dma_stream());
        DMA_InitTypeDef dma_init;

        dma_init.DMA_Channel            = config::tx_dma_channel();
        dma_init.DMA_PeripheralBaseAddr = (uint32_t)config::spi() + 0x0C;

        dma_init.DMA_Memory0BaseAddr    = reinterpret_cast<uint32_t>(buf);
        dma_init.DMA_BufferSize         = buf_size;
        dma_init.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
        dma_init.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word;

        dma_init.DMA_DIR                = DMA_DIR_MemoryToPeripheral;
        dma_init.DMA_PeripheralInc      = DMA_PeripheralInc_Disable;
        dma_init.DMA_MemoryInc          = DMA_MemoryInc_Enable;
        dma_init.DMA_Mode               = DMA_Mode_Normal;
        dma_init.DMA_Priority           = DMA_Priority_Medium;
        dma_init.DMA_FIFOMode           = DMA_FIFOMode_Disable;
        dma_init.DMA_FIFOThreshold      = DMA_FIFOThreshold_3QuartersFull;
        dma_init.DMA_MemoryBurst        = DMA_MemoryBurst_Single;
        dma_init.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single;
        DMA_Init(config::tx_dma_stream(), &dma_init);

        SPI_DMACmd(config::spi(), SPI_I2S_DMAReq_Tx, ENABLE);

        DMA_ITConfig(config::tx_dma_stream(), DMA_IT_TC | DMA_IT_HT , ENABLE);
        NVIC_InitTypeDef NVIC_InitStructure;

        /* Enable the SPI Interrupt */
        NVIC_InitStructure.NVIC_IRQChannel                   = config::tx_dma_IRQ_channel();
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
        NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }

    void check()
    {
        log("SPI3 st:  %d\n", getState());
        log("SPI3 CR1: 0x%04x\n", config::spi()->CR1);
        log("SPI3 CR2: 0x%04x\n", config::spi()->CR2);
        log("SPI3 SR:  0x%04x\n", config::spi()->SR);
        log("DMA1 LISR: 0x%08x\n", DMA1->LISR);
        log("DMA1 HISR: 0x%08x\n", DMA1->HISR);
    }

    void dmaLog()
    {
#ifdef DEBUG_SPI
        log("dma_RX_IRQ_handler()\n");
        log("DMA1 LISR: 0x%08x\n", DMA1->LISR);
        log("DMA1 HISR: 0x%08x\n", DMA1->HISR);
#endif
    }

    void dma_RX_IRQ_handler()
    {
        dmaLog();

        if( DMA_GetITStatus(config::rx_dma_stream(), config::rx_dma_int_HT() ) )
        {
            DMA_ClearITPendingBit(config::rx_dma_stream(), config::rx_dma_int_HT());
            transferFlags |= SPI_RX_HALF;
        }
        if (DMA_GetITStatus(config::rx_dma_stream(), config::rx_dma_int_TC()))
        {
            DMA_ClearITPendingBit(config::rx_dma_stream(), config::rx_dma_int_TC());
            transferFlags |= SPI_RX_DONE;

            DMA_Cmd(config::rx_dma_stream(), DISABLE);
            dmaComplete();
        }
    }

    void dma_TX_IRQ_handler()
    {
        dmaLog();

        if (DMA_GetITStatus(config::tx_dma_stream(), config::tx_dma_int_HT()))
        {
            DMA_ClearITPendingBit(config::tx_dma_stream(), config::tx_dma_int_HT());
            transferFlags |= SPI_TX_HALF;
        }
        if (DMA_GetITStatus(config::tx_dma_stream(), config::tx_dma_int_TC()))
        {
            DMA_ClearITPendingBit(config::tx_dma_stream(), config::tx_dma_int_TC());
            transferFlags |= SPI_TX_DONE;

            DMA_Cmd(config::tx_dma_stream(), DISABLE);
            dmaComplete();
        }
    }

    void dmaComplete()
    {
        if ((transferFlags & SPI_TX_DONE) &&
            (transferFlags & SPI_RX_DONE))
        {
            disable();
            state = SPI_STATE_DONE;
        }
    }


    bool isTransferActive() {
        return (state == SPI_STATE_WAIT_NO_SS) || (state == SPI_STATE_ACTIVE);
    }
    bool isTransferComplete() {
        return (state == SPI_STATE_DONE);
    }

    uint32_t getState() const {
        return state;
    }
    void setState(uint32_t st) {
        AtomicBlock AB;
        state = st;
    }



private:
    volatile uint32_t state;
    volatile uint32_t transferFlags;
};

#endif // SPI_BASE_HPP
