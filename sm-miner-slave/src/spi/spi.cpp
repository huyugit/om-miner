/*******************************************************************************
*  file    : spi.cpp
*  created : 23.05.2013
*  author  : Slyshyk Oleksiy (alexSlyshyk@gmail.com)
*******************************************************************************/

#include "spi.hpp"
#include "atomic_block.hpp"
#include "format.hpp"


//-----------------------------------------------
#if defined(USE_BRD_MB_V1)
//-----------------------------------------------

Spi1 spi1;

// SPI1_RX: DMA2: Stream0/Channel_3, Stream2/Channel_3
// SPI1_TX: DMA2: Stream3/Channel_3, Stream5/Channel_3

extern "C" void SPI1_IRQHandler()
{
    log("SPI1_IRQHandler()\n");
}

extern "C" void DMA2_Stream0_IRQHandler(void)
{
    spi1.dma_RX_IRQ_handler();
}
extern "C" void DMA2_Stream3_IRQHandler(void)
{
    spi1.dma_TX_IRQ_handler();
}

extern "C" void EXTI15_10_IRQHandler(void)
{
    //log("EXTI15_10_IRQHandler()\n");
    spi1.spi_NSS_interrupt();
}

//-----------------------------------------------
#elif defined(USE_BRD_F4_DISCOVERY)
//-----------------------------------------------

Spi3 spi3;

extern "C" void SPI3_IRQHandler()
{
    log("SPI3_IRQHandler()\n");
}

extern "C" void DMA1_Stream0_IRQHandler(void)
{
    spi3.dma_RX_IRQ_handler();
}
extern "C" void DMA1_Stream5_IRQHandler(void)
{
    spi3.dma_TX_IRQ_handler();
}

extern "C" void EXTI4_IRQHandler(void)
{
    //log("EXTI4_IRQHandler()\n");
    spi3.spi_NSS_interrupt();
}

//-----------------------------------------------
#else
#error "Must define board revision !"
#endif
//-----------------------------------------------







