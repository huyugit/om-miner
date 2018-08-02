/*******************************************************************************
*  file    : atomic_block.hpp
*  created : 16.10.2012
*  author  : Slyshyk Oleksiy (alex312@meta.ua)
*******************************************************************************/

#ifndef ATOMIC_BLOCK_HPP
#define ATOMIC_BLOCK_HPP

#ifdef STM32F4XX
#include <stm32f4xx.h>
#elif defined(STM32f10X)
#include <stm32f10x.h>
#endif

inline uint32_t __get_primask(void)
{
    uint32_t result=0;
    __asm volatile ("MRS %0, primask" : "=r" (result) );
    return(result);
}

inline void __set_primask(uint32_t priMask)
{
    __asm volatile ("MSR primask, %0" : : "r" (priMask) );
}

class AtomicBlock
{
public:
    AtomicBlock()
    {
        sreg_ = __get_primask();
        __disable_irq();
    }
    ~AtomicBlock()
    {
        __set_primask(sreg_);
    }
private:
    uint32_t sreg_;
};




#endif // ATOMIC_BLOCK_HPP
