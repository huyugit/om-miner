#ifndef IPWRCHECK_H
#define IPWRCHECK_H

#include "stm32f4xx_can.h"

class IPwrCheck
{
public:
	virtual bool autoCheckModule(void){return false;};
};
#endif
