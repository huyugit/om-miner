#include "uart_config.hpp"


UartConfig::UartConfig(int _id, GPIO_Pins _txPin, GPIO_Pins _rxPin, DMA_Stream_TypeDef *_txDmaStream, uint32_t _txDmaChannel, DMA_Stream_TypeDef *_rxDmaStream, uint32_t _rxDmaChannel)
    :
      id(_id),
      txPin(_txPin), rxPin(_rxPin),
      txDmaStream(_txDmaStream), txDmaChannel(_txDmaChannel),
      rxDmaStream(_rxDmaStream), rxDmaChannel(_rxDmaChannel)
{
    switch (id)
    {
    case 1: usart = USART1; break;
    case 2: usart = USART2; break;
    case 3: usart = USART3; break;
    case 4: usart = UART4;  break;
    case 5: usart = UART5;  break;
    case 6: usart = USART6; break;
    }

    switch (id)
    {
    case 1: usartIRQ = USART1_IRQn; break;
    case 2: usartIRQ = USART2_IRQn; break;
    case 3: usartIRQ = USART3_IRQn; break;
    case 4: usartIRQ = UART4_IRQn;  break;
    case 5: usartIRQ = UART5_IRQn;  break;
    case 6: usartIRQ = USART6_IRQn; break;
    }

    txDmaStreamIRQ = dmaStreamIRQ(txDmaStream);
    rxDmaStreamIRQ = dmaStreamIRQ(rxDmaStream);

    txDmaIntHT = dmaIntHT(txDmaStream);
    txDmaIntTC = dmaIntTC(txDmaStream);

    rxDmaIntHT = dmaIntHT(rxDmaStream);
    rxDmaIntTC = dmaIntTC(rxDmaStream);
}

void UartConfig::configureHw()
{
    // enable USART module clock
    enableUartPeriphClock();

    // enable USART pins clock
    RCC_AHB1PeriphClockCmd(StmGPIO::gpioToPeriph(txPin), ENABLE);
    RCC_AHB1PeriphClockCmd(StmGPIO::gpioToPeriph(rxPin), ENABLE);

    // connect USART pins to USART module
    initPinAF(txPin);
    initPinAF(rxPin);

    initPins();
}

void UartConfig::enableUartPeriphClock()
{
    switch (id)
    {
    case 1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); break;
    case 2: RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); break;
    case 3: RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); break;
    case 4: RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,  ENABLE); break;
    case 5: RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,  ENABLE); break;
    case 6: RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE); break;
    }
}

void UartConfig::initPinAF(GPIO_Pins pin)
{
    switch (id)
    {
    case 1: GPIO_PinAFConfig(StmGPIO::gpioToGpioTypeDef(pin), StmGPIO::gpioToPin(pin), GPIO_AF_USART1); break;
    case 2: GPIO_PinAFConfig(StmGPIO::gpioToGpioTypeDef(pin), StmGPIO::gpioToPin(pin), GPIO_AF_USART2); break;
    case 3: GPIO_PinAFConfig(StmGPIO::gpioToGpioTypeDef(pin), StmGPIO::gpioToPin(pin), GPIO_AF_USART3); break;
    case 4: GPIO_PinAFConfig(StmGPIO::gpioToGpioTypeDef(pin), StmGPIO::gpioToPin(pin), GPIO_AF_UART4);  break;
    case 5: GPIO_PinAFConfig(StmGPIO::gpioToGpioTypeDef(pin), StmGPIO::gpioToPin(pin), GPIO_AF_UART5);  break;
    case 6: GPIO_PinAFConfig(StmGPIO::gpioToGpioTypeDef(pin), StmGPIO::gpioToPin(pin), GPIO_AF_USART6); break;
    }
}

void UartConfig::initPins()
{
    GPIO_InitTypeDef gpio_init;
    // Configure  TX  as alternate function push-pull
    gpio_init.GPIO_Pin   = (1 << StmGPIO::gpioToPin(txPin));
    gpio_init.GPIO_Speed = GPIO_Speed_2MHz;
    gpio_init.GPIO_Mode  = GPIO_Mode_AF;
    gpio_init.GPIO_OType = GPIO_OType_PP;
    gpio_init.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(StmGPIO::gpioToGpioTypeDef(txPin), &gpio_init);
    // Configure RX  as input floating
    gpio_init.GPIO_Pin   = (1 << StmGPIO::gpioToPin(rxPin));
    gpio_init.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(StmGPIO::gpioToGpioTypeDef(rxPin), &gpio_init);
}

void UartConfig::enableDmaClock()
{
    enableDmaClock(txDmaStream);
    enableDmaClock(rxDmaStream);
}

void UartConfig::enableDmaClock(DMA_Stream_TypeDef *ds)
{
    if (ds == DMA1_Stream0 ||
            ds == DMA1_Stream1 ||
            ds == DMA1_Stream2 ||
            ds == DMA1_Stream3 ||
            ds == DMA1_Stream4 ||
            ds == DMA1_Stream5 ||
            ds == DMA1_Stream6 ||
            ds == DMA1_Stream7)
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
        return;
    }

    if (ds == DMA2_Stream0 ||
            ds == DMA2_Stream1 ||
            ds == DMA2_Stream2 ||
            ds == DMA2_Stream3 ||
            ds == DMA2_Stream4 ||
            ds == DMA2_Stream5 ||
            ds == DMA2_Stream6 ||
            ds == DMA2_Stream7)
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
        return;
    }
}

uint8_t UartConfig::dmaStreamIRQ(DMA_Stream_TypeDef *ds)
{
    if (ds == DMA1_Stream0) return DMA1_Stream0_IRQn;
    if (ds == DMA1_Stream0) return DMA1_Stream0_IRQn;
    if (ds == DMA1_Stream1) return DMA1_Stream1_IRQn;
    if (ds == DMA1_Stream2) return DMA1_Stream2_IRQn;
    if (ds == DMA1_Stream3) return DMA1_Stream3_IRQn;
    if (ds == DMA1_Stream4) return DMA1_Stream4_IRQn;
    if (ds == DMA1_Stream5) return DMA1_Stream5_IRQn;
    if (ds == DMA1_Stream6) return DMA1_Stream6_IRQn;
    if (ds == DMA1_Stream7) return DMA1_Stream7_IRQn;

    if (ds == DMA2_Stream0) return DMA2_Stream0_IRQn;
    if (ds == DMA2_Stream1) return DMA2_Stream1_IRQn;
    if (ds == DMA2_Stream2) return DMA2_Stream2_IRQn;
    if (ds == DMA2_Stream3) return DMA2_Stream3_IRQn;
    if (ds == DMA2_Stream4) return DMA2_Stream4_IRQn;
    if (ds == DMA2_Stream5) return DMA2_Stream5_IRQn;
    if (ds == DMA2_Stream6) return DMA2_Stream6_IRQn;
    if (ds == DMA2_Stream7) return DMA2_Stream7_IRQn;

    return 0;
}

uint32_t UartConfig::dmaIntHT(DMA_Stream_TypeDef *ds)
{
    if (ds == DMA1_Stream0 || ds == DMA2_Stream0) return DMA_IT_HTIF0;
    if (ds == DMA1_Stream1 || ds == DMA2_Stream1) return DMA_IT_HTIF1;
    if (ds == DMA1_Stream2 || ds == DMA2_Stream2) return DMA_IT_HTIF2;
    if (ds == DMA1_Stream3 || ds == DMA2_Stream3) return DMA_IT_HTIF3;
    if (ds == DMA1_Stream4 || ds == DMA2_Stream4) return DMA_IT_HTIF4;
    if (ds == DMA1_Stream5 || ds == DMA2_Stream5) return DMA_IT_HTIF5;
    if (ds == DMA1_Stream6 || ds == DMA2_Stream6) return DMA_IT_HTIF6;
    if (ds == DMA1_Stream7 || ds == DMA2_Stream7) return DMA_IT_HTIF7;

    return 0;
}

uint32_t UartConfig::dmaIntTC(DMA_Stream_TypeDef *ds)
{
    if (ds == DMA1_Stream0 || ds == DMA2_Stream0) return DMA_IT_TCIF0;
    if (ds == DMA1_Stream1 || ds == DMA2_Stream1) return DMA_IT_TCIF1;
    if (ds == DMA1_Stream2 || ds == DMA2_Stream2) return DMA_IT_TCIF2;
    if (ds == DMA1_Stream3 || ds == DMA2_Stream3) return DMA_IT_TCIF3;
    if (ds == DMA1_Stream4 || ds == DMA2_Stream4) return DMA_IT_TCIF4;
    if (ds == DMA1_Stream5 || ds == DMA2_Stream5) return DMA_IT_TCIF5;
    if (ds == DMA1_Stream6 || ds == DMA2_Stream6) return DMA_IT_TCIF6;
    if (ds == DMA1_Stream7 || ds == DMA2_Stream7) return DMA_IT_TCIF7;

    return 0;
}
