/*******************************************************************************
*  file    :
*  created : 21.03.2015
*  author  : Slyshyk Oleksiy
*******************************************************************************/

#ifndef ELTEK_RECTIFIER_H
#define ELTEK_RECTIFIER_H

#include <cstdint>

#include "eltek_msg.h"
#include "ms_data.h"


enum {
    ELTEK_TURN_ON = 0,

    ELTEK_ALARM_MINOR,
    ELTEK_ALARM_MAJOR,

    ELTEK_PROD_DESC,
    ELTEK_PROD_PART,
    ELTEK_PROD_VER,
    ELTEK_PROD_YEAR,
    ELTEK_PROD_MONTH,
    ELTEK_PROD_DAY,

    ELTEK_GREEN_LED,
    ELTEK_YELLOW_LED,
    ELTEK_RED_LED,

    ELTEK_UP_TIME,
    ELTEK_FAN_SPPED,
    ELTEK_FAN_SPPED_REF,

    ELTEK_DATA_NUM
};

class RequestsQueue
{
public:
    RequestsQueue();

    uint8_t size() const;
    bool isEmpty() const;

    void push(uint8_t id);
    bool pop(uint8_t &id);

    void clear(uint8_t id);

//private:
    uint8_t queue[16];
    uint32_t mask;

    uint32_t head, tail;
};


struct EltekRectifier
{
    EltekRectifier();

    PsuInfo info;
    PsuSpec spec;

    bool isPsuHe;
    bool isPsuHeDc1;

    uint32_t  seqErrors;
    EltekGenMessage msgSeq;

    RequestsQueue requests;

    uint32_t reqLedsTime;

    void        onMinorAlarm(uint16_t x);
    void        onMajorAlarm(uint16_t x);

    void        onDefaultVoltage(uint16_t x);

    void        onGreenLed(bool x);
    void        onYellowLed(bool x);
    void        onRedLed(bool x);

    void        onUpTime(uint32_t x);
    void        onFanSpeedRef(uint16_t x);
    void        onFanSpeed(uint16_t x);

    void        onProdDesc      (char*);
    void        onProdPart      (char*);
    void        onProdVer       (char*);
    void        onProdYear      (uint16_t);
    void        onProdMonth     (uint8_t);
    void        onProdDay       (uint8_t);

};

#endif // ELTEK_RECTIFIER_H
