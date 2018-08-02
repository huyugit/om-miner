#ifndef UART_HPP
#define UART_HPP

#include "uart_base.hpp"
#include "uart_config.hpp"


extern UartBase uart1;
extern UartBase uart3;
extern UartBase uart4;


//-----------------------------------------------
#if defined(USE_BRD_F4_DISCOVERY)
//-----------------------------------------------

#define uartDebug uart3
#define uartDebug2 uart4

//-----------------------------------------------
#else
//-----------------------------------------------

#define uartDebug uart1
#define uartDebug2 uart4

//-----------------------------------------------
#endif
//-----------------------------------------------



#endif // UART_HPP
