/*******************************************************************************
*  file    :
*  created : 02.07.2018
*  author  :
*******************************************************************************/

#include "gpio_exti.h"
#ifdef MACH_A3
int g_fan_fault_cnt = 0;
int g_hash0_vfalut_cnt = 0;
int g_hash0_ifalut_cnt = 0;
int g_hash1_vfalut_cnt = 0;
int g_hash1_ifalut_cnt = 0;
int g_hash2_vfalut_cnt = 0;
int g_hash2_ifalut_cnt = 0;
#endif

void GpioExti::init()
{
    nvicConfig();
}

void GpioExti::nvicConfig()
{
    NVIC_InitTypeDef  nvicInit;
	EXTI_InitTypeDef EXTI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // Move to config
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0); // fan fault
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource3); // hash0 vfault
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource4); // hash0 ifault
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource3); // hash1 vfault
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4); // hash1 ifault
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource3); // hash2 vfault
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource4); // hash2 ifault
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; // fan fault
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line3; // hash0/1/2 vfault
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	EXTI_InitStructure.EXTI_Line = EXTI_Line4; // hash0/1/2 ifault
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Fan Fault PA0 */
    nvicInit.NVIC_IRQChannel = EXTI0_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x0;
    nvicInit.NVIC_IRQChannelSubPriority = 0x0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

	/* VFault Hash0/1/2 PA3 PB3 PC3 */
    nvicInit.NVIC_IRQChannel = EXTI3_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x0;
    nvicInit.NVIC_IRQChannelSubPriority = 0x0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);

	/* IFault Hash0/1/2 PA4 PB4 PC4 */
    nvicInit.NVIC_IRQChannel = EXTI4_IRQn;
    nvicInit.NVIC_IRQChannelPreemptionPriority = 0x0;
    nvicInit.NVIC_IRQChannelSubPriority = 0x0;
    nvicInit.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvicInit);
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f40xx.s/startup_stm32f427x.s).                         */
/******************************************************************************/

extern "C" void EXTI0_IRQHandler(void)
{
    g_fan_fault_cnt += 1;
	EXTI_ClearITPendingBit(EXTI_Line0);
}

extern "C" void EXTI3_IRQHandler(void)
{
	g_hash0_vfalut_cnt += 1;
	//g_hash1_vfalut_cnt += 1;
	//g_hash2_vfalut_cnt += 1;
	EXTI_ClearITPendingBit(EXTI_Line3);
}

extern "C" void EXTI4_IRQHandler(void)
{
	g_hash0_ifalut_cnt += 1;
	//g_hash1_ifalut_cnt += 1;
	//g_hash2_ifalut_cnt += 1;

    EXTI_ClearITPendingBit(EXTI_Line4);
}


