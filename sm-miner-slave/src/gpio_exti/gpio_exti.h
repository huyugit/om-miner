/*******************************************************************************
*  file    :
*  created : 02.07.2018
*  author  : gezhihua
*******************************************************************************/

#ifndef GPIO_EXTI_HPP
#define GPIO_EXTI_HPP

#include <cstdint>
#include "stm32f4xx_gpio.h"

class GpioExti
{
public:
    static void init();

private:
    static void nvicConfig();
};

#ifdef MACH_A3
extern int g_fan_fault_cnt;
extern int g_hash0_vfalut_cnt;
extern int g_hash0_ifalut_cnt;
extern int g_hash1_vfalut_cnt;
extern int g_hash1_ifalut_cnt;
extern int g_hash2_vfalut_cnt;
extern int g_hash2_ifalut_cnt;
#endif
#endif // CAN_HPP

