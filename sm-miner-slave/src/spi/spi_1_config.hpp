#ifndef SPI_1_CONFIG_HPP
#define SPI_1_CONFIG_HPP

#include "stm32f4xx.h"
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_spi.h>


struct spi_1_config
{
    // SPI1: AHB1/APB2
    // SPI1_NSS:  PA15 (software)
    // SPI1_SCK:  PA5, PB3
    // SPI1_MISI: PA6, PB4
    // SPI1_MOSI: PA7, PB5
    // SPI1_RX: DMA2: Stream0/Channel_3, Stream2/Channel_3
    // SPI1_TX: DMA2: Stream3/Channel_3, Stream5/Channel_3

    static inline SPI_TypeDef* spi() {return SPI1;}

    static inline uint8_t              spi_IRQ_channel      (){return SPI1_IRQn;}
    static        void                 enable_dma_clk       (){RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);}

    static inline DMA_Stream_TypeDef * rx_dma_stream        (){return DMA2_Stream0;}
    static inline uint32_t             rx_dma_channel       (){return DMA_Channel_3;}
    static inline uint32_t             rx_dma_int_TC        (){return DMA_IT_TCIF0;}
    static inline uint32_t             rx_dma_int_HT        (){return DMA_IT_HTIF0;}
    static inline uint8_t              rx_dma_IRQ_channel   (){return DMA2_Stream0_IRQn;}

    static inline DMA_Stream_TypeDef * tx_dma_stream        (){return DMA2_Stream3;}
    static inline uint32_t             tx_dma_channel       (){return DMA_Channel_3;}
    static inline uint32_t             tx_dma_int_TC        (){return DMA_IT_TCIF3;}
    static inline uint32_t             tx_dma_int_HT        (){return DMA_IT_HTIF3;}
    static inline uint8_t              tx_dma_IRQ_channel   (){return DMA2_Stream3_IRQn;}

    static inline GPIO_TypeDef*        nss_GPIO             (){return GPIOA;}
    static inline uint16_t             nss_GPIO_Pin         (){return GPIO_Pin_15;}
    static inline uint32_t             nss_EXTI_Line        (){return EXTI_Line15;}

    static void init_hw()
    {
        log("SPI1: init\n");
        init_gpio();
        init_spi_as_SPI_slave();
        init_spi_NSS_interrupt();
    }

private:
    static void init_gpio()
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1  , ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA , ENABLE);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource15,GPIO_AF_SPI1);

        GPIO_InitTypeDef GPIO_InitStructure;

        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
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

        SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource15); // Move to config

        EXTI_InitTypeDef EXTI_InitStructure;
        EXTI_InitStructure.EXTI_Line = EXTI_Line15; // Move to config
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);

        NVIC_InitTypeDef NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn; // Move to config
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
};


#endif // SPI_1_CONFIG_HPP
