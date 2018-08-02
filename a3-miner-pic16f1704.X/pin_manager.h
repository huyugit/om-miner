/**
  @Generated Pin Manager Header File

  @Company:
    Microchip Technology Inc.

  @File Name:
    pin_manager.h

  @Summary:
    This is the Pin Manager file generated using MPLAB(c) Code Configurator

  @Description:
    This header file provides implementations for pin APIs for all pins selected in the GUI.
    Generation Information :
        Product Revision  :  MPLAB(c) Code Configurator - 4.35
        Device            :  PIC16F1704
        Version           :  1.01
    The generated drivers are tested against the following:
        Compiler          :  XC8 1.35
        MPLAB             :  MPLAB X 3.40

    Copyright (c) 2013 - 2015 released Microchip Technology Inc.  All rights reserved.

    Microchip licenses to you the right to use, modify, copy and distribute
    Software only when embedded on a Microchip microcontroller or digital signal
    controller that is integrated into your product or third party product
    (pursuant to the sublicense terms in the accompanying license agreement).

    You should refer to the license agreement accompanying this Software for
    additional information regarding your rights and obligations.

    SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
    EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
    MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
    IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
    CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
    OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
    INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
    CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
    SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
    (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

*/


#ifndef PIN_MANAGER_H
#define PIN_MANAGER_H

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set IFault_ADC aliases
#define IFault_ADC_TRIS               TRISAbits.TRISA2
#define IFault_ADC_LAT                LATAbits.LATA2
#define IFault_ADC_PORT               PORTAbits.RA2
#define IFault_ADC_WPU                WPUAbits.WPUA2
#define IFault_ADC_OD                ODCONAbits.ODA2
#define IFault_ADC_ANS                ANSELAbits.ANSA2
#define IFault_ADC_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define IFault_ADC_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define IFault_ADC_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define IFault_ADC_GetValue()           PORTAbits.RA2
#define IFault_ADC_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define IFault_ADC_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define IFault_ADC_SetPullup()      do { WPUAbits.WPUA2 = 1; } while(0)
#define IFault_ADC_ResetPullup()    do { WPUAbits.WPUA2 = 0; } while(0)
#define IFault_ADC_SetPushPull()    do { ODCONAbits.ODA2 = 0; } while(0)
#define IFault_ADC_SetOpenDrain()   do { ODCONAbits.ODA2 = 1; } while(0)
#define IFault_ADC_SetAnalogMode()  do { ANSELAbits.ANSA2 = 1; } while(0)
#define IFault_ADC_SetDigitalMode() do { ANSELAbits.ANSA2 = 0; } while(0)

// get/set VFault_ADC aliases
#define VFault_ADC_TRIS               TRISAbits.TRISA4
#define VFault_ADC_LAT                LATAbits.LATA4
#define VFault_ADC_PORT               PORTAbits.RA4
#define VFault_ADC_WPU                WPUAbits.WPUA4
#define VFault_ADC_OD                ODCONAbits.ODA4
#define VFault_ADC_ANS                ANSELAbits.ANSA4
#define VFault_ADC_SetHigh()            do { LATAbits.LATA4 = 1; } while(0)
#define VFault_ADC_SetLow()             do { LATAbits.LATA4 = 0; } while(0)
#define VFault_ADC_Toggle()             do { LATAbits.LATA4 = ~LATAbits.LATA4; } while(0)
#define VFault_ADC_GetValue()           PORTAbits.RA4
#define VFault_ADC_SetDigitalInput()    do { TRISAbits.TRISA4 = 1; } while(0)
#define VFault_ADC_SetDigitalOutput()   do { TRISAbits.TRISA4 = 0; } while(0)
#define VFault_ADC_SetPullup()      do { WPUAbits.WPUA4 = 1; } while(0)
#define VFault_ADC_ResetPullup()    do { WPUAbits.WPUA4 = 0; } while(0)
#define VFault_ADC_SetPushPull()    do { ODCONAbits.ODA4 = 0; } while(0)
#define VFault_ADC_SetOpenDrain()   do { ODCONAbits.ODA4 = 1; } while(0)
#define VFault_ADC_SetAnalogMode()  do { ANSELAbits.ANSA4 = 1; } while(0)
#define VFault_ADC_SetDigitalMode() do { ANSELAbits.ANSA4 = 0; } while(0)

// get/set LCE aliases
#define LCE_TRIS               TRISAbits.TRISA5
#define LCE_LAT                LATAbits.LATA5
#define LCE_PORT               PORTAbits.RA5
#define LCE_WPU                WPUAbits.WPUA5
#define LCE_OD                ODCONAbits.ODA5
#define LCE_SetHigh()            do { LATAbits.LATA5 = 1; } while(0)
#define LCE_SetLow()             do { LATAbits.LATA5 = 0; } while(0)
#define LCE_Toggle()             do { LATAbits.LATA5 = ~LATAbits.LATA5; } while(0)
#define LCE_GetValue()           PORTAbits.RA5
#define LCE_SetDigitalInput()    do { TRISAbits.TRISA5 = 1; } while(0)
#define LCE_SetDigitalOutput()   do { TRISAbits.TRISA5 = 0; } while(0)
#define LCE_SetPullup()      do { WPUAbits.WPUA5 = 1; } while(0)
#define LCE_ResetPullup()    do { WPUAbits.WPUA5 = 0; } while(0)
#define LCE_SetPushPull()    do { ODCONAbits.ODA5 = 0; } while(0)
#define LCE_SetOpenDrain()   do { ODCONAbits.ODA5 = 1; } while(0)

// get/set SCL aliases
#define SCL_TRIS               TRISCbits.TRISC0
#define SCL_LAT                LATCbits.LATC0
#define SCL_PORT               PORTCbits.RC0
#define SCL_WPU                WPUCbits.WPUC0
#define SCL_OD                ODCONCbits.ODC0
#define SCL_ANS                ANSELCbits.ANSC0
#define SCL_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define SCL_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define SCL_Toggle()             do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define SCL_GetValue()           PORTCbits.RC0
#define SCL_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define SCL_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)
#define SCL_SetPullup()      do { WPUCbits.WPUC0 = 1; } while(0)
#define SCL_ResetPullup()    do { WPUCbits.WPUC0 = 0; } while(0)
#define SCL_SetPushPull()    do { ODCONCbits.ODC0 = 0; } while(0)
#define SCL_SetOpenDrain()   do { ODCONCbits.ODC0 = 1; } while(0)
#define SCL_SetAnalogMode()  do { ANSELCbits.ANSC0 = 1; } while(0)
#define SCL_SetDigitalMode() do { ANSELCbits.ANSC0 = 0; } while(0)

// get/set SDA aliases
#define SDA_TRIS               TRISCbits.TRISC1
#define SDA_LAT                LATCbits.LATC1
#define SDA_PORT               PORTCbits.RC1
#define SDA_WPU                WPUCbits.WPUC1
#define SDA_OD                ODCONCbits.ODC1
#define SDA_ANS                ANSELCbits.ANSC1
#define SDA_SetHigh()            do { LATCbits.LATC1 = 1; } while(0)
#define SDA_SetLow()             do { LATCbits.LATC1 = 0; } while(0)
#define SDA_Toggle()             do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define SDA_GetValue()           PORTCbits.RC1
#define SDA_SetDigitalInput()    do { TRISCbits.TRISC1 = 1; } while(0)
#define SDA_SetDigitalOutput()   do { TRISCbits.TRISC1 = 0; } while(0)
#define SDA_SetPullup()      do { WPUCbits.WPUC1 = 1; } while(0)
#define SDA_ResetPullup()    do { WPUCbits.WPUC1 = 0; } while(0)
#define SDA_SetPushPull()    do { ODCONCbits.ODC1 = 0; } while(0)
#define SDA_SetOpenDrain()   do { ODCONCbits.ODC1 = 1; } while(0)
#define SDA_SetAnalogMode()  do { ANSELCbits.ANSC1 = 1; } while(0)
#define SDA_SetDigitalMode() do { ANSELCbits.ANSC1 = 0; } while(0)

// get/set RC2 procedures
#define RC2_SetHigh()    do { LATCbits.LATC2 = 1; } while(0)
#define RC2_SetLow()   do { LATCbits.LATC2 = 0; } while(0)
#define RC2_Toggle()   do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define RC2_GetValue()         PORTCbits.RC2
#define RC2_SetDigitalInput()   do { TRISCbits.TRISC2 = 1; } while(0)
#define RC2_SetDigitalOutput()  do { TRISCbits.TRISC2 = 0; } while(0)
#define RC2_SetPullup()     do { WPUCbits.WPUC2 = 1; } while(0)
#define RC2_ResetPullup()   do { WPUCbits.WPUC2 = 0; } while(0)
#define RC2_SetAnalogMode() do { ANSELCbits.ANSC2 = 1; } while(0)
#define RC2_SetDigitalMode()do { ANSELCbits.ANSC2 = 0; } while(0)

// get/set RC4 procedures
#define RC4_SetHigh()    do { LATCbits.LATC4 = 1; } while(0)
#define RC4_SetLow()   do { LATCbits.LATC4 = 0; } while(0)
#define RC4_Toggle()   do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define RC4_GetValue()         PORTCbits.RC4
#define RC4_SetDigitalInput()   do { TRISCbits.TRISC4 = 1; } while(0)
#define RC4_SetDigitalOutput()  do { TRISCbits.TRISC4 = 0; } while(0)
#define RC4_SetPullup()     do { WPUCbits.WPUC4 = 1; } while(0)
#define RC4_ResetPullup()   do { WPUCbits.WPUC4 = 0; } while(0)
#define RC4_SetAnalogMode() do { ANSELCbits.ANSC4 = 1; } while(0)
#define RC4_SetDigitalMode()do { ANSELCbits.ANSC4 = 0; } while(0)

// get/set RC5 procedures
#define RC5_SetHigh()    do { LATCbits.LATC5 = 1; } while(0)
#define RC5_SetLow()   do { LATCbits.LATC5 = 0; } while(0)
#define RC5_Toggle()   do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define RC5_GetValue()         PORTCbits.RC5
#define RC5_SetDigitalInput()   do { TRISCbits.TRISC5 = 1; } while(0)
#define RC5_SetDigitalOutput()  do { TRISCbits.TRISC5 = 0; } while(0)
#define RC5_SetPullup()     do { WPUCbits.WPUC5 = 1; } while(0)
#define RC5_ResetPullup()   do { WPUCbits.WPUC5 = 0; } while(0)
#define RC5_SetAnalogMode() do { ANSELCbits.ANSC5 = 1; } while(0)
#define RC5_SetDigitalMode()do { ANSELCbits.ANSC5 = 0; } while(0)

/**
   @Param
    none
   @Returns
    none
   @Description
    GPIO and peripheral I/O initialization
   @Example
    PIN_MANAGER_Initialize();
 */
void PIN_MANAGER_Initialize (void);

/**
 * @Param
    none
 * @Returns
    none
 * @Description
    Interrupt on Change Handling routine
 * @Example
    PIN_MANAGER_IOC();
 */
void PIN_MANAGER_IOC(void);



#endif // PIN_MANAGER_H
/**
 End of File
*/