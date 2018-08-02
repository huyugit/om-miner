#ifndef CMN_DEFINES_H
#define CMN_DEFINES_H

#include <stdint.h>

#define DEFAULT_DIFF            1024

#define MAX_BTC16_PER_PWC       11

#define EXTRA_NONCE_BYTES       8
#define HASH_BYTES              32      // 8 words
#define COINBASE_BYTES          256     // 64 words
#define MERKLE_ROOT_SIZE        16


//  CLK     CPU     UART    SER     SER
//  CFG     MHz      CFG    DIV     MHz
//    ?      20      195
//    ?      50      434
//    0      82      711
//   32     136     1180     16     8.0
//   40     165     1440     32     5.0
//   42     172     1450     32     5.2


// PWC 41 MHz
//#define PWC_CPU_CLOCK_CFG       0x80
//#define PWC_CPU_CLOCK_MHZ       41

// PWC 82 MHz
//#define PWC_CPU_CLOCK_CFG       0
//#define PWC_CPU_CLOCK_MHZ       82

// PWC 136 MHz
#define PWC_CPU_CLOCK_CFG       32
#define PWC_CPU_CLOCK_MHZ       136

// PWC 165 MHz
//#define PWC_CPU_CLOCK_CFG       40
//#define PWC_CPU_CLOCK_MHZ       165

// PWC 172 MHz
//#define PWC_CPU_CLOCK_CFG       42
//#define PWC_CPU_CLOCK_MHZ       172


// auto calculate uart speed cfg
#define PWC_UART_SPEED_CFG      (PWC_CPU_CLOCK_MHZ*1000*1000 / 230400)

// auto calculate btc serial cfg
#define BTC_SER_MHZ             8
#define BTC_SER_DIV             (PWC_CPU_CLOCK_MHZ / BTC_SER_MHZ)
#define BTC16_SER_CONFIG        (((BTC_SER_DIV - 1) << 16) | (BTC_SER_DIV))


#define PWC_EZ_ADDR             0x00000020

#define PWC_ERR_NONE            0xAAAAAAAA

#define SPI_LEN 	12
#define PWR_LEN 	12
#define ADCU_FAC	15.78
#define ADCI_FAC	10.00
#define MACH_DESC	"48.8V 1L12x8"
#define FAN_NUM		6
#define FAN_CONTROLER	"max31790"
#define FAN_CTRL_NAME_LEN	8
#define SLAVE_COUNT_DFT 	4
#define ADC_TO_U	4.03
#define ADC_TO_I	10.3
#define MAX_PSU_PER_SLAVE        2
#define MAX_PWC_PER_SPI          12
#define FAN_I_MAX	20
#define FAN_U_MAX	13
#define FAN_COUNT 	6
#define PWR_I_MAX	60
#define PWR_U_MAX	52
#define PWR_P_MAX	2950

/* lxj add begin 20180328 */
#define MAX_SLAVE_ERROR_NUMS          12 //1 modify to 12£¬by huyu
/* lxj add end */

#endif // CMN_DEFINES_H
