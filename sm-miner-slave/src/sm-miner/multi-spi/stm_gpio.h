#ifndef STM_GPIO_H
#define STM_GPIO_H

#include <stdint.h>
#include "stm32f4xx.h"

//=======================================================

enum GPIO_Direction
{
  kINPUT	= 0,
  kOUTPUT   = 1
};

enum GPIO_Pins {
    PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15,
    PB0, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
    PC0, PC1, PC2, PC3, PC4, PC5, PC6, PC7, PC8, PC9, PC10, PC11, PC12, PC13, PC14, PC15,
    PD0, PD1, PD2, PD3, PD4, PD5, PD6, PD7, PD8, PD9, PD10, PD11, PD12, PD13, PD14, PD15,
    PE0, PE1, PE2, PE3, PE4, PE5, PE6, PE7, PE8, PE9, PE10, PE11, PE12, PE13, PE14, PE15,
    PF0, PF1, PF2, PF3, PF4, PF5, PF6, PF7, PF8, PF9, PF10, PF11, PF12, PF13, PF14, PF15,
    PH0, PH1, PH2, PH3, PH4, PH5, PH6, PH7, PH8, PH9, PH10, PH11, PH12, PH13, PH14, PH15,
    PI0, PI1, PI2, PI3, PI4, PI5, PI6, PI7, PI8, PI9, PI10, PI11, PI12, PI13, PI14, PI15,
    PJ0, PJ1, PJ2, PJ3, PJ4, PJ5, PJ6, PJ7, PJ8, PJ9, PJ10, PJ11, PJ12, PJ13, PJ14, PJ15,
};

// temp - not connected
#define PXX (PA1)

 
enum GPIO_Ports {
    GPIO_PORT_A, GPIO_PORT_B, GPIO_PORT_C, GPIO_PORT_D, GPIO_PORT_E,
    GPIO_PORT_F, GPIO_PORT_G, GPIO_PORT_H, GPIO_PORT_I, GPIO_PORT_J,
};

enum GPIO_Bits {
    GPIO_PIN_0,  GPIO_PIN_1,  GPIO_PIN_2,  GPIO_PIN_3,  GPIO_PIN_4,  GPIO_PIN_5,
    GPIO_PIN_6,  GPIO_PIN_7,  GPIO_PIN_8,  GPIO_PIN_9,  GPIO_PIN_10, GPIO_PIN_11,
    GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};

//=======================================================

class StmGPIO
{

public:
    StmGPIO();
    ~StmGPIO();

public:
    static uint8_t gpioToPort(uint8_t gpio);
    static uint8_t gpioToPin(uint8_t gpio);
    static GPIO_TypeDef* portToGpioTypeDef(uint8_t port);
    static GPIO_TypeDef* gpioToGpioTypeDef(uint8_t gpio);
    static uint32_t gpioToPeriph(uint8_t gpio);

    // Configure pin as input/output
    bool configurePin(uint8_t _pin, GPIO_Direction _direction);

	// Enable/Disable interrupts for the pin
    bool enablePinInterrupts(uint8_t _pin, bool _enable);

	// Write a value to a pin
    void writePin(uint8_t _pin, uint8_t _value);

	// Write set of values to pins
    void writePinArray(int _num, uint8_t _pins[], uint32_t _values);
  
	// Read a value from a pin
    uint8_t readPin(uint8_t _pin);

    // Read set of values from pins
    void readPinArray(int _num, uint8_t _pins[], uint8_t _values[]);

    void* precalcFastWritePinArray(int _num, uint8_t _pins[]);
    void fastWritePinArray(void* cachePtr, uint8_t value);

private:
	int			        m_gpio_fd;
	unsigned long *		m_controlModule;
};

extern StmGPIO g_StmGPIO;

#endif // STM_GPIO_H
 
