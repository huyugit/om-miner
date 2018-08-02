/*******************************************************************************
*  file    :
*  created : 10.03.2015
*  author  :
*******************************************************************************/

#ifndef CAN_HPP
#define CAN_HPP

#include <cstdint>
#include "array.hpp"
#include "stm32f4xx_can.h"

class Can
{
public:
    static void init();
    static void send(CanTxMsg msg);
    static void send2(CanTxMsg msg);

    static void sendTxBuffer();
    static void rxIrqHandler();
    static void errIrqHandler();

    static uint32_t rxMsgSize() {return rxBuffer.size();}
    static CanRxMsg rxPop() {return rxBuffer.pop_back();}

private:
    static RingBuffer<CanRxMsg,100> rxBuffer;
    static RingBuffer<CanTxMsg,100> txBuffer;

    static void nvicConfig();
    static void canConfig();
};

#endif // CAN_HPP
