/**
  OPA1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    opa1.c

  @Summary
    This is the generated driver implementation file for the OPA1 driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This source file provides implementations for driver APIs for OPA1.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.65
        Device            :  PIC16F1704
        Driver Version    :  2.00
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.45
        MPLAB             :  MPLAB X 4.10
*/

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/
#ifndef _ADDA_H
#define _ADDA_H

/**
  Section: Included Files
*/

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

#define _XTAL_FREQ 16000000

typedef uint16_t adc_result_t;

typedef enum
{
	IFault_ADC =  0x2,
	VFault_ADC =  0x3,
	channel_FVRBuffer2 =  0x1C,
	channel_Temp =	0x1D,
	channel_DAC =  0x1E,
	channel_FVRBuffer1 =  0x1F
} adc_channel_t;

/**
  Section: OPA1 APIs
*/

void OPA1_Initialize(void);

void DAC_Initialize(void);

void DAC_SetOutput(uint8_t inputData);
/**
  Section: ADC Module APIs
*/

void ADC_Initialize(void);

void ADC_SelectChannel(adc_channel_t channel);

void ADC_StartConversion();

bool ADC_IsConversionDone();

adc_result_t ADC_GetConversionResult(void);

adc_result_t ADC_GetConversion(adc_channel_t channel);

void ADC_TemperatureAcquisitionDelay(void);

/* ADDA operate API */
adc_result_t get_adc_value(adc_channel_t channel);

void set_dac_value(adc_result_t dac);

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif
/**
 End of File
*/

