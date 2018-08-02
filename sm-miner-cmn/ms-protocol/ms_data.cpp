#include "ms_data.h"

namespace {
const double MB50V_ADC_K = 18.3; // mV/adc
const double MB50V_ADC_B = 0.0;
}


double SlaveMbInfo::getMbVoltage() const
{
    return getMbVoltageMV() / 1000.0;
}

uint32_t SlaveMbInfo::getMbVoltageMV() const
{
    return MB50V_ADC_K * adc50V + MB50V_ADC_B;
}
