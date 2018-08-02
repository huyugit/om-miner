#include "stm_gpio.h"

#include "stm32f4xx_rcc.h"
#include "format.hpp"


StmGPIO g_StmGPIO;

//=======================================================

uint8_t StmGPIO::gpioToPort(uint8_t gpio)
{
    return (gpio >> 4) & 0x7;
}


uint8_t StmGPIO::gpioToPin(uint8_t gpio)
{
    return gpio & 0xf;
}

#define MAX_PORTS 9
GPIO_TypeDef* portToGpioTypeDefMapping[MAX_PORTS] = {
    GPIOA, GPIOB, GPIOC, GPIOD, GPIOE,
    GPIOF, GPIOG, GPIOH, GPIOI
};

GPIO_TypeDef* StmGPIO::portToGpioTypeDef(uint8_t port)
{
    if (port >= MAX_PORTS) {
        log("SPI: wrong gpio port\n"); STOP();
    }
    return portToGpioTypeDefMapping[port];
}
GPIO_TypeDef* StmGPIO::gpioToGpioTypeDef(uint8_t gpio)
{
    return portToGpioTypeDef(gpioToPort(gpio));
}

uint32_t StmGPIO::gpioToPeriph(uint8_t gpio)
{
    // Return value accordingly to this mapping:
    // #define RCC_AHB1Periph_GPIOA             ((uint32_t)0x00000001)
    // #define RCC_AHB1Periph_GPIOB             ((uint32_t)0x00000002)
    // #define RCC_AHB1Periph_GPIOC             ((uint32_t)0x00000004)
    // #define RCC_AHB1Periph_GPIOD             ((uint32_t)0x00000008)
    // #define RCC_AHB1Periph_GPIOE             ((uint32_t)0x00000010)
    // #define RCC_AHB1Periph_GPIOF             ((uint32_t)0x00000020)
    // #define RCC_AHB1Periph_GPIOG             ((uint32_t)0x00000040)
    // #define RCC_AHB1Periph_GPIOH             ((uint32_t)0x00000080)
    // #define RCC_AHB1Periph_GPIOI             ((uint32_t)0x00000100)

    return 1 << gpioToPort(gpio);
}

//=======================================================

StmGPIO::StmGPIO()
{
}

StmGPIO::~StmGPIO()
{
}
 

// Configure pin as input/output
bool StmGPIO::configurePin(uint8_t _pin, GPIO_Direction _direction)
{
    // Enable clock for port
    RCC_AHB1PeriphClockCmd(gpioToPeriph(_pin), ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_StructInit(&GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = (1 << gpioToPin(_pin));
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

    if (_direction == kINPUT)
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    else
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;

    //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    //GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;

    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

//    log("Init port:\n");
//    log("  GPIO_Pin:      %d\n", GPIO_InitStructure.GPIO_Pin);
//    log("  GPIO_Mode:     %d\n", GPIO_InitStructure.GPIO_Mode);
//    log("  GPIO_Speed:    %d\n", GPIO_InitStructure.GPIO_Speed);
//    log("  GPIO_OType:    %d\n", GPIO_InitStructure.GPIO_OType);
//    log("  GPIO_PuPd:     %d\n", GPIO_InitStructure.GPIO_PuPd);
//    log("  GPIO:          0x%08x\n", gpioToGpioTypeDef(_pin));

    GPIO_Init(gpioToGpioTypeDef(_pin), &GPIO_InitStructure);
    return true;
}


// Enable/Disable interrupts for the pin
bool StmGPIO::enablePinInterrupts(uint8_t _pin, bool _enable)
{
    log("SPI: not implemented\n"); STOP();
	_pin = _pin;
	_enable = _enable;
	return true;
}


// Write a value to a pin
void StmGPIO::writePin(uint8_t _pin, uint8_t _value)
{
    uint8_t bit = gpioToPin(_pin);
    uint32_t mask = 0x1 << bit;

    //printf("CLR/SET BIT: base addr = 0x%08x, offset = 0x%08x, clr mask = 0x%08x, set mask = 0x%08x\n", GPIO_Pin_Bank[_pin], kDATAOUT/4, mask, v);
    //printf("writePin: BEFORE: 0x%08x\n", g_gpio[GPIO_Pin_Bank[_pin]][kDATAOUT/4]);
  
    if (_value) {
        // set bit
        GPIO_WriteBit(gpioToGpioTypeDef(_pin), mask, Bit_SET);
    }
    else {
        // clear bit
        GPIO_WriteBit(gpioToGpioTypeDef(_pin), mask, Bit_RESET);
    }

    //printf("writePin: AFTER:  0x%08x\n", g_gpio[GPIO_Pin_Bank[_pin]][kDATAOUT/4]);
}


void StmGPIO::writePinArray(int _num, uint8_t _pins[], uint32_t _values)
{
	for (uint8_t i = 0; i < _num; i++)
	{
		int value = _values & 0x1;
		_values >>= 1;

        int pin = _pins[i];
        int bank = gpioToPort(pin);
        int bit = gpioToPin(pin);

        uint32_t v_mask = (0x1 << bit);

        if (!value) {
            GPIO_ResetBits(portToGpioTypeDef(bank), v_mask);
        }
        else {
            GPIO_SetBits(portToGpioTypeDef(bank), v_mask);
        }
	}
}
 

// Read a value from a pin
uint8_t StmGPIO::readPin(uint8_t _pin)
{
    uint8_t bit = gpioToPin(_pin);
    uint16_t mask = 1 << bit;
    return GPIO_ReadInputDataBit(gpioToGpioTypeDef(_pin), mask);
}
 
void StmGPIO::readPinArray(int _num, uint8_t _pins[], uint8_t _values[])
{
	// prepare masks for each bank
	for (uint8_t i = 0; i < _num; i++)
	{
        int gpio = _pins[i];
        int bit = gpioToPin(gpio);
        uint32_t mask = (0x1 << bit);
        
        _values[i] = GPIO_ReadInputDataBit(gpioToGpioTypeDef(gpio), mask);;
	}
}

//=======================================================

struct CachedPortMask
{
    GPIO_TypeDef* port;
    uint16_t mask;
    bool isLast;
};

class CachedPortManager
{
public:
    static CachedPortMask& allocate()
    {
        numCachedPortMask++;

        if (numCachedPortMask > MAX_CACHED_PORT_MASKS) {
            log("GPIO: can not allocate more CachedPortMask structures\n"); STOP();
        }

        log("GPIO: allocated %d structures\n", numCachedPortMask);
        return cachedPortMask[numCachedPortMask-1];
    }

private:
    static int numCachedPortMask;

    static const int MAX_CACHED_PORT_MASKS = 16;
    static CachedPortMask cachedPortMask[MAX_CACHED_PORT_MASKS];
};

int CachedPortManager::numCachedPortMask = 0;
CachedPortMask CachedPortManager::cachedPortMask[MAX_CACHED_PORT_MASKS];


void* StmGPIO::precalcFastWritePinArray(int _num, uint8_t _pins[])
{
    // clean and set mask for each bank
    uint32_t port_mask[MAX_PORTS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    // prepare masks for each bank
    for (uint8_t i = 0; i < _num; i++)
    {
        int pin = _pins[i];
        int bank = gpioToPort(pin);
        int bit = gpioToPin(pin);

        port_mask[bank] |= (0x1 << bit);
    }

    // write to each bank
    CachedPortMask* result = nullptr;
    CachedPortMask* current = nullptr;

    for (uint8_t bank = 0; bank < MAX_PORTS; bank++)
    {
        if (port_mask[bank] == 0) continue;

        if (current) {
            current->isLast = false;
        }

        current = &CachedPortManager::allocate();
        current->port = portToGpioTypeDef(bank);
        current->mask = port_mask[bank];
        current->isLast = true;

        if (!result) {
            result = current;
        }
    }

    return result;
}

void StmGPIO::fastWritePinArray(void* cachePtr, uint8_t value)
{
    CachedPortMask* current = (CachedPortMask*)cachePtr;
    while (current)
    {
        if (!value)
            GPIO_ResetBits(current->port, current->mask);
        else
            GPIO_SetBits(current->port, current->mask);

        if (current->isLast) break;
        current++;
    }
}
