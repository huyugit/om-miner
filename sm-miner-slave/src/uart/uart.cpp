#include "uart.hpp"
#include "format.hpp"


UartBase uart1(UartConfigFactory::get1());
UartBase uart3(UartConfigFactory::get3());
UartBase uart4(UartConfigFactory::get4());


// debug

extern "C" void DMA2_Stream7_IRQHandler(void)
{
    uart1.dma_TX_IRQ_handler();
}
extern "C" void DMA2_Stream2_IRQHandler(void)
{
    uart1.dma_RX_IRQ_handler();
}

// uartMdb0

extern "C" void DMA1_Stream3_IRQHandler(void)
{
    uart3.dma_TX_IRQ_handler();
}
extern "C" void DMA1_Stream1_IRQHandler(void)
{
    uart3.dma_RX_IRQ_handler();
}

// uartMdb1

extern "C" void DMA1_Stream4_IRQHandler(void)
{
    uart4.dma_TX_IRQ_handler();
}
extern "C" void DMA1_Stream2_IRQHandler(void)
{
    uart4.dma_RX_IRQ_handler();
}
