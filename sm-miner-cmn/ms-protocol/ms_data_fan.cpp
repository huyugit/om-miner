#include "ms_data_fan.h"

namespace {
// volt range: min V (dac 4095) .. max V (dac 0)
const double DAC_K = -0.00188; // V/dac
const double DAC_B = 15.83; // V
}


uint16_t FanConfig::fanVoltToDac(double volt)
{
    int dac = (volt - DAC_B) / DAC_K;

    if (dac < 0) dac = 0;
    if (dac > 4095) dac = 4095;

    return dac;
}

double FanConfig::fanDacToVolt(uint16_t dac)
{
    return DAC_K * dac + DAC_B;
}

namespace {
const double K_ADC_TO_U = ADC_TO_U / 1000;  // mV / ADC
const double K_ADC_TO_I = ADC_TO_I / 1000;  // mA / ADC
}

double FanInfo::getFanU() const {
    return (double)fanAdcU * K_ADC_TO_U;
}

double FanInfo::getFanI() const {
    return (double)fanAdcI * K_ADC_TO_I;
}
