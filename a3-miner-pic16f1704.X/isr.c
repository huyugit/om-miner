#include <stdint.h>
#include <xc.h>

#include "usart_pic16.h"

void interrupt ISR(void)
{
    if (RCIE && RCIF) {
        USARTHandleRxInt();
        return;
    }
}