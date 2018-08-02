#ifndef SPI_3_CONFIG_HPP
#define SPI_3_CONFIG_HPP

#include "stm32f4xx.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_spi.h>


struct spi_3_config
{
    // SPI3: AHB1/APB1
    // SPI3_NSS:  PA4, PA15
    // SPI3_SCK:  PB3, PC10
    // SPI3_MISO: PB4, PC11
    // SPI3_MOSI: PB5, PC12
    // SPI3_RX: DMA1: Stream0/Channel_0, Stream2/Channel_0
    // SPI3_TX: DMA1: Stream5/Channel_0, Stream7/Channel_0

    static inline SPI_TypeDef* spi() {return SPI3;}

    static inline uint8_t              spi_IRQ_channel      (){return SPI3_IRQn;}
    static        void                 enable_dma_clk       (){RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);}

    static inline DMA_Stream_TypeDef * rx_dma_stream        (){return DMA1_Stream0;}
    static inline uint32_t             rx_dma_channel       (){return DMA_Channel_0;}
    static inline uint32_t             rx_dma_int_TC        (){return DMA_IT_TCIF0;}
    static inline uint32_t             rx_dma_int_HT        (){return DMA_IT_HTIF0;}
    static inline uint8_t              rx_dma_IRQ_channel   (){return DMA1_Stream0_IRQn;}

    static inline DMA_Stream_TypeDef * tx_dma_stream        (){return DMA1_Stream5;}
    static inline uint32_t             tx_dma_channel       (){return DMA_Channel_0;}
    static inline uint32_t             tx_dma_int_TC        (){return DMA_IT_TCIF5;}
    static inline uint32_t             tx_dma_int_HT        (){return DMA_IT_HTIF5;}
    static inline uint8_t              tx_dma_IRQ_channel   (){return DMA1_Stream5_IRQn;}

    static inline GPIO_TypeDef*        nss_GPIO             (){return GPIOA;}
    static inline uint16_t             nss_GPIO_Pin         (){return GPIO_Pin_4;}
    static inline uint32_t             nss_EXTI_Line        (){return EXTI_Line4;}

    static void init_hw()
    {
        init_gpio();
        init_spi_as_SPI_slave();
        init_spi_NSS_interrupt();
    }

private:
    static void init_gpio()
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3  , ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB , ENABLE);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_SPI3);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI3);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI3);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI3);

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
    }

    static void init_spi_as_SPI_slave()
    {
        SPI_InitTypeDef  SPI_InitStructure;

        SPI_I2S_DeInit(spi());
        SPI_InitStructure.SPI_Mode              = SPI_Mode_Slave;
        SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
        SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
        SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;              //точка захвата данных
        SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;
        SPI_InitStructure.SPI_NSS               = SPI_NSS_Hard;
        SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
        SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
        SPI_InitStructure.SPI_CRCPolynomial     = 7;

        SPI_Init(spi(), &SPI_InitStructure);
    }

    static void init_spi_NSS_interrupt()
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // Move to config

        SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); // Move to config

        EXTI_InitTypeDef EXTI_InitStructure;
        EXTI_InitStructure.EXTI_Line = EXTI_Line4; // Move to config
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn; // Move to config
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
};


#endif // SPI_3_CONFIG_HPP
