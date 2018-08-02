#ifndef UART_CONFIG_HPP
#define UART_CONFIG_HPP

#include <stdint.h>
#include <stm32f4xx_usart.h>
#include <static_assert.h>
#include <stm_gpio.h>


struct UartConfig
{
    int                 id;
    GPIO_Pins           txPin;
    GPIO_Pins           rxPin;

    USART_TypeDef*      usart;
    uint8_t             usartIRQ;

    DMA_Stream_TypeDef* txDmaStream;
    uint8_t             txDmaStreamIRQ;
    uint32_t            txDmaChannel;
    uint32_t            txDmaIntHT;
    uint32_t            txDmaIntTC;

    DMA_Stream_TypeDef* rxDmaStream;
    uint8_t             rxDmaStreamIRQ;
    uint32_t            rxDmaChannel;
    uint32_t            rxDmaIntHT;
    uint32_t            rxDmaIntTC;


    UartConfig(int _id,
               GPIO_Pins _txPin, GPIO_Pins _rxPin,
               DMA_Stream_TypeDef* _txDmaStream, uint32_t _txDmaChannel,
               DMA_Stream_TypeDef* _rxDmaStream, uint32_t _rxDmaChannel);

    void configureHw();
    void enableUartPeriphClock();
    void initPinAF(GPIO_Pins pin);
    void initPins();
    void enableDmaClock();
    static void enableDmaClock(DMA_Stream_TypeDef* ds);
    static uint8_t dmaStreamIRQ(DMA_Stream_TypeDef* ds);
    static uint32_t dmaIntHT(DMA_Stream_TypeDef* ds);
    static uint32_t dmaIntTC(DMA_Stream_TypeDef* ds);
};


struct UartConfigFactory
{
    static UartConfig get1() {
        return UartConfig(1, PA9, PA10,
                          DMA2_Stream7, DMA_Channel_4,
                          DMA2_Stream2, DMA_Channel_4); }
    static UartConfig get2() {
        return UartConfig(2, PA2, PA3,
                          DMA1_Stream6, DMA_Channel_4,
                          DMA1_Stream5, DMA_Channel_4); }
    static UartConfig get3() {
        return UartConfig(3, PB10, PB11,
                          DMA1_Stream3, DMA_Channel_4,
                          DMA1_Stream1, DMA_Channel_4); }
    static UartConfig get4() {
        return UartConfig(4, PC10, PC11,
                          DMA1_Stream4, DMA_Channel_4,
                          DMA1_Stream2, DMA_Channel_4); }
    static UartConfig get5() {
        return UartConfig(5, PC12, PD2,
                          DMA1_Stream7, DMA_Channel_4,
                          DMA1_Stream0, DMA_Channel_4); }
    static UartConfig get6() {
        return UartConfig(6, PC6, PC7,
                          DMA2_Stream6, DMA_Channel_5,
                          DMA2_Stream1, DMA_Channel_5); }
};

#endif // UART_CONFIG_HPP
