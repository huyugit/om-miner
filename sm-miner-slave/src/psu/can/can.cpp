/*******************************************************************************
*  file    :
*  created : 10.03.2015
*  author  :
*******************************************************************************/

#include "can.h"
#include "format.hpp"


#if defined(USE_BRD_F4_DISCOVERY)

// CAN RX/TX: PB8/PB9
#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_8
#define CAN_TX_PIN                 GPIO_Pin_9
#define CAN_GPIO_PORT              GPIOB
#define CAN_GPIO_CLK               RCC_AHB1Periph_GPIOB
#define CAN_AF_PORT                GPIO_AF_CAN1
#define CAN_RX_SOURCE              GPIO_PinSource8
#define CAN_TX_SOURCE              GPIO_PinSource9

#else

// CAN RX/TX: PA11/PA12
#define CANx                       CAN1
#define CAN_CLK                    RCC_APB1Periph_CAN1
#define CAN_RX_PIN                 GPIO_Pin_11
#define CAN_TX_PIN                 GPIO_Pin_12
#define CAN_GPIO_PORT              GPIOA
#define CAN_GPIO_CLK               RCC_AHB1Periph_GPIOA
#define CAN_AF_PORT                GPIO_AF_CAN1
#define CAN_RX_SOURCE              GPIO_PinSource11
#define CAN_TX_SOURCE              GPIO_PinSource12

//#define CANx                       CAN1
//#define CAN_CLK                    RCC_APB1Periph_CAN1
//#define CAN_RX_PIN                 GPIO_Pin_8
//#define CAN_TX_PIN                 GPIO_Pin_9
//#define CAN_GPIO_PORT              GPIOB
//#define CAN_GPIO_CLK               RCC_AHB1Periph_GPIOB
//#define CAN_AF_PORT                GPIO_AF_CAN1
//#define CAN_RX_SOURCE              GPIO_PinSource8
//#define CAN_TX_SOURCE              GPIO_PinSource9

#endif


RingBuffer<CanRxMsg,100> Can::rxBuffer;
RingBuffer<CanTxMsg,100> Can::txBuffer;


void Can::init()
{
    canConfig();
    nvicConfig();
}

void Can::send(CanTxMsg msg)
{
    if (txBuffer.is_full()) {
        log("Can::send: txBuffer full!\n");
    }

    txBuffer.push_back(msg);
    Can::sendTxBuffer();
}

void Can::send2(CanTxMsg msg)
{
	msg.StdId = msg.StdId;
//    txBuffer.push_back(msg);
//    Can::sendTxBuffer();
}

void Can::sendTxBuffer()
{
    while (txBuffer.size())
    {
        if (CAN_Transmit(CANx, &txBuffer.front()) == CAN_TxStatus_NoMailBox)
        {
            return;
        }

        txBuffer.pop_front();
    }
}

void Can::rxIrqHandler()
{
    CanRxMsg msg;
    CAN_Receive(CAN1, CAN_FIFO0, &msg);
	rxBuffer.push_back(msg);
}

void Can::errIrqHandler()
{
    /**
      * @brief  Checks whether the specified CAN flag is set or not.
      * @param  CANx: where x can be 1 or 2 to to select the CAN peripheral.
      * @param  CAN_FLAG: specifies the flag to check.
      *          This parameter can be one of the following values:
      *            @arg CAN_FLAG_RQCP0: Request MailBox0 Flag
      *            @arg CAN_FLAG_RQCP1: Request MailBox1 Flag
      *            @arg CAN_FLAG_RQCP2: Request MailBox2 Flag
      *            @arg CAN_FLAG_FMP0: FIFO 0 Message Pending Flag
      *            @arg CAN_FLAG_FF0: FIFO 0 Full Flag
      *            @arg CAN_FLAG_FOV0: FIFO 0 Overrun Flag
      *            @arg CAN_FLAG_FMP1: FIFO 1 Message Pending Flag
      *            @arg CAN_FLAG_FF1: FIFO 1 Full Flag
      *            @arg CAN_FLAG_FOV1: FIFO 1 Overrun Flag
      *            @arg CAN_FLAG_WKU: Wake up Flag
      *            @arg CAN_FLAG_SLAK: Sleep acknowledge Flag
      *            @arg CAN_FLAG_EWG: Error Warning Flag
      *            @arg CAN_FLAG_EPV: Error Passive Flag
      *            @arg CAN_FLAG_BOF: Bus-Off Flag
      *            @arg CAN_FLAG_LEC: Last error code Flag
      * @retval The new state of CAN_FLAG (SET or RESET).
      */

    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_EWG))
    {
        log("Error Warning Flag\n");
        CAN_ClearFlag(CAN1, CAN_FLAG_EWG);
    }
}

void Can::nvicConfig()
{
    NVIC_InitTypeDef  nvicInit;

    nvicInit.NVIC_IRQChannel = CAN1_TX_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x0;
    nvicInit.NVIC_IRQChannelSubPriority = 0x0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

    nvicInit.NVIC_IRQChannel = CAN1_RX0_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x0;
    nvicInit.NVIC_IRQChannelSubPriority = 0x0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

    nvicInit.NVIC_IRQChannel = CAN1_SCE_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x1;
    nvicInit.NVIC_IRQChannelSubPriority = 0x0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);
}

void Can::canConfig()
{
    GPIO_InitTypeDef  gpioInit;

    /* CAN GPIOs configuration **************************************************/

    /* Enable GPIO clock */
    RCC_AHB1PeriphClockCmd(CAN_GPIO_CLK, ENABLE);

    /* Connect CAN pins to AF9 */
    GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_RX_SOURCE, CAN_AF_PORT);
    GPIO_PinAFConfig(CAN_GPIO_PORT, CAN_TX_SOURCE, CAN_AF_PORT);

    /* Configure CAN RX and TX pins */
    gpioInit.GPIO_Pin = CAN_RX_PIN | CAN_TX_PIN;
    gpioInit.GPIO_Mode = GPIO_Mode_AF;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    gpioInit.GPIO_OType = GPIO_OType_PP;
    gpioInit.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(CAN_GPIO_PORT, &gpioInit);

    /* CAN configuration ********************************************************/
    /* Enable CAN clock */
    RCC_APB1PeriphClockCmd(CAN_CLK, ENABLE);

    /* CAN register init */
    CAN_DeInit(CANx);

    /* CAN cell init */
    CAN_InitTypeDef canInit;
    canInit.CAN_TTCM = DISABLE;
    canInit.CAN_ABOM = ENABLE;
    canInit.CAN_AWUM = ENABLE;
    canInit.CAN_NART = DISABLE;
    canInit.CAN_RFLM = DISABLE;
    canInit.CAN_TXFP = DISABLE;
    canInit.CAN_Mode = CAN_Mode_Normal;
    canInit.CAN_SJW  = CAN_SJW_1tq;

    /* CAN Baudrate = 125 KBps  */
    canInit.CAN_BS1          = CAN_BS1_10tq;
    canInit.CAN_BS2          = CAN_BS2_5tq;
    canInit.CAN_Prescaler    = 21; //for 168 MHz
    //CAN_InitStructure_.CAN_Prescaler = 8;

    CAN_Init(CANx, &canInit);


    /* CAN filter init */
    CAN_FilterInitTypeDef filterInit;
    filterInit.CAN_FilterNumber           = 0;
    filterInit.CAN_FilterMode             = CAN_FilterMode_IdMask;
    filterInit.CAN_FilterScale            = CAN_FilterScale_32bit;
    filterInit.CAN_FilterIdHigh           = 0x0000;
    filterInit.CAN_FilterIdLow            = 0x0000;
    filterInit.CAN_FilterMaskIdHigh       = 0x0000;
    filterInit.CAN_FilterMaskIdLow        = 0x0000;
    filterInit.CAN_FilterFIFOAssignment   = 0;
    filterInit.CAN_FilterActivation       = ENABLE;
    CAN_FilterInit(&filterInit);


    /* Enable FIFO 0 message pending Interrupt */
    CAN_ITConfig(CANx, CAN_IT_FMP0, ENABLE);
    CAN_ITConfig(CANx, CAN_IT_TME, ENABLE);
}


/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s/startup_stm32f427x.s).                         */
/******************************************************************************/

extern "C" void CAN1_TX_IRQHandler(void)
{
    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP0)) CAN_ClearFlag(CAN1, CAN_FLAG_RQCP0);
    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP1)) CAN_ClearFlag(CAN1, CAN_FLAG_RQCP1);
    if (CAN_GetFlagStatus(CAN1, CAN_FLAG_RQCP2)) CAN_ClearFlag(CAN1, CAN_FLAG_RQCP2);

    Can::sendTxBuffer();
}

extern "C" void CAN1_RX0_IRQHandler(void)
{
    Can::rxIrqHandler();
}

extern "C" void CAN1_SCE_IRQHandler(void)
{
    Can::errIrqHandler();
}
