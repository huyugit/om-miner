#ifndef MS_DATA_FAN_H
#define MS_DATA_FAN_H

#include <stdint.h>
#include "ms_defines.h"
#include "format.hpp"


struct FanConfig
{
    uint16_t fanDac;

    static const uint16_t FAN_DAC_MIN = 1500; // U < 13.0V
    static const uint16_t FAN_DAC_MAX = 3600; // U > 9.0V

    FanConfig()
        : fanDac(2048)
    {}

    double getFanVoltage() const {
        return fanDacToVolt(fanDac);
    }
    void setFanVoltage(double volt) {
        fanDac = fanVoltToDac(volt);
    }

    void setFanMin() {
        fanDac = 4095;
    }
    void setFanMax() {
        fanDac = 0;
    }

    static uint16_t fanVoltToDac(double volt);
    static double fanDacToVolt(uint16_t dac);

    void dump()
    {
        log("FanConfig: dac 0x%03x\n", fanDac);
    }
}
__attribute__ ((packed));


struct FanInfo
{
    uint16_t fanAdcU;
    uint16_t fanAdcI;

    FanInfo()
        : fanAdcU(0),
          fanAdcI(0)
    {}

    double getFanU() const;
    double getFanI() const;

    void dump()
    {
        log("FanInfo: adcU %u, adcI %u\n", fanAdcU, fanAdcI);
    }
}
__attribute__ ((packed));


struct FanTestInfo
{
    FanConfig cfg;
    FanInfo info;

    FanTestInfo()
    {}

    void dump()
    {
        log("FanTestInfo:\n");
        log("  cfg:  "); cfg.dump();
        log("  info: "); info.dump();
    }
}
__attribute__ ((packed));


struct FanTestArr
{
    static const size_t NUM = 3;
    FanTestInfo items[NUM];

    void dump()
    {
        log("FanTestArr:\n");
        for (size_t i = 0; i < NUM; i++) {
            log("  item[%d]: ", i); items[i].dump();
        }
    }
}
__attribute__ ((packed));

#endif // MS_DATA_FAN_H
