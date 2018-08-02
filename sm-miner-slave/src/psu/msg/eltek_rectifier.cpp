/*******************************************************************************
*  file    :
*  created : 21.03.2015
*  author  : Slyshyk Oleksiy
*******************************************************************************/

#include <cstring>
#include "eltek_mgr.h"
#include "eltek_rectifier.h"
#include "format.hpp"


RequestsQueue::RequestsQueue()
    : mask(0), head(0), tail(0)
{}

uint8_t RequestsQueue::size() const
{
    int r = head - tail;
    if (r < 0) r += 16;
    return r;
}

bool RequestsQueue::isEmpty() const
{
    return head == tail;
}

void RequestsQueue::push(uint8_t id)
{
    // already in queue
    if (mask & (1 << id)) return;

    uint32_t head2 = (head + 1) % 16;

    // overflow
    if (head2 == tail) return;

    queue[head] = id;
    head = head2;

    mask |= (1 << id);
}

bool RequestsQueue::pop(uint8_t &id)
{
    // empty queue
    if (head == tail) return false;

    id = queue[tail];
    tail = (tail + 1) % 16;

    mask &= ~(1 << id); // clear
    return true;
}

void RequestsQueue::clear(uint8_t id)
{
    mask &= ~(1 << id);

    uint32_t i = tail;
    uint32_t ii = i;

    while (ii != head)
    {
        queue[i] = queue[ii];

        ii = (ii + 1) % 16;

        if (queue[i] != id)
        {
            i = (i + 1) % 16;
        }
    }

    head = i;
}


EltekRectifier::EltekRectifier()
    : isPsuHe(false), isPsuHeDc1(false),
      seqErrors(0),
      reqLedsTime(0)
{
    memset(&info, 0, sizeof(info));

    for (int i = 0; i < ELTEK_DATA_NUM; i++)
    {
        if (i != ELTEK_TURN_ON)
        {
            requests.push(i);
        }
    }

    spec.prodDesc[0] = 0;
    spec.prodPart[0] = 0;
    spec.prodVer[0] = 0;
}

void EltekRectifier::onMinorAlarm(uint16_t x)
{
    info.minorAlarm = x;
    if (EltekMgr::debugOn) log("PSU[%u]: minorAlarm: %u\n", info.id, info.minorAlarm);
    requests.clear(ELTEK_ALARM_MINOR);
}

void EltekRectifier::onMajorAlarm(uint16_t x)
{
    info.majorAlarm = x;
    if (EltekMgr::debugOn) log("PSU[%u]: majorAlarm: %u\n", info.id, info.majorAlarm);
    requests.clear(ELTEK_ALARM_MAJOR);
}

void EltekRectifier::onDefaultVoltage(uint16_t x)
{
    info.defaultVoltage = x;
    if (EltekMgr::debugOn) log("PSU[%u]: defaultVoltage: %u\n", info.id, info.defaultVoltage);
    requests.clear(ELTEK_ALARM_MAJOR);
}

void EltekRectifier::onGreenLed(bool x)
{
    info.greenLed = x;
    if (EltekMgr::debugOn) log("PSU[%u]: greenLed: %u\n", info.id, info.greenLed);
    requests.clear(ELTEK_GREEN_LED);
}

void EltekRectifier::onYellowLed(bool x)
{
    info.yellowLed = x;
    if (EltekMgr::debugOn) log("PSU[%u]: yellowLed: %u\n", info.id, info.yellowLed);
    requests.clear(ELTEK_YELLOW_LED);
}

void EltekRectifier::onRedLed(bool x)
{
    info.redLed = x;
    if (EltekMgr::debugOn) log("PSU[%u]: redLed: %u\n", info.id, info.redLed);
    requests.clear(ELTEK_RED_LED);
}

void EltekRectifier::onUpTime(uint32_t x)
{
    info.upTime = x;
    if (EltekMgr::debugOn) log("PSU[%u]: upTime: %u\n", info.id, info.upTime);
    requests.clear(ELTEK_UP_TIME);
}

void EltekRectifier::onFanSpeedRef(uint16_t x)
{
    info.fanSpeedRef = x;
    if (EltekMgr::debugOn) log("PSU[%u]: fanSpeedRef: %u\n", info.id, info.fanSpeedRef);
    requests.clear(ELTEK_FAN_SPPED_REF);
}

void EltekRectifier::onFanSpeed(uint16_t x)
{
    info.fanSpeed = x;
    if (EltekMgr::debugOn) log("PSU[%u]: fanSpeed: %u\n", info.id, info.fanSpeed);
    requests.clear(ELTEK_FAN_SPPED);
}


void EltekRectifier::onProdDesc(char *desc)
{
    uint32_t copy_sz = ::strlen(desc) > 26 ? 26 : ::strlen(desc);
    ::memcpy(spec.prodDesc, desc, copy_sz);
    spec.prodDesc[copy_sz] = 0;

    isPsuHe = (strcmp(spec.prodDesc, "FLATPACK2 48/3000 HE") == 0);
    isPsuHeDc1 = (strcmp(spec.prodDesc, "FLATPACK2 48/3000 HE DC1") == 0);

    if (EltekMgr::debugOn) log("PSU[%u]: prodDesc: %s\n", info.id, spec.prodDesc);
    requests.clear(ELTEK_PROD_DESC);
}

void EltekRectifier::onProdPart(char *part)
{
    uint32_t copy_sz = ::strlen(part) > 11 ? 11 : ::strlen(part);
    ::memcpy(spec.prodPart,part, copy_sz);
    spec.prodPart[copy_sz] = 0;

    if (EltekMgr::debugOn) log("PSU[%u]: prodPart: %s\n", info.id, spec.prodPart);
    requests.clear(ELTEK_PROD_PART);
}

void EltekRectifier::onProdVer(char *ver)
{
    uint32_t copy_sz = ::strlen(ver) > 5 ? 5 : ::strlen(ver);
    ::memcpy(spec.prodVer,ver, copy_sz);
    spec.prodVer[copy_sz] = 0;

    if (EltekMgr::debugOn) log("PSU[%u]: prodVer: %s\n", info.id, spec.prodVer);
    requests.clear(ELTEK_PROD_VER);
}

void EltekRectifier::onProdYear(uint16_t y)
{
    spec.prodYear = y;
    if (EltekMgr::debugOn) log("PSU[%u]: prodYear: %u\n", info.id, spec.prodYear);
    requests.clear(ELTEK_PROD_YEAR);
}

void EltekRectifier::onProdMonth(uint8_t m)
{
    spec.prodMonth = m;
    if (EltekMgr::debugOn) log("PSU[%u]: prodMonth: %u\n", info.id, spec.prodMonth);
    requests.clear(ELTEK_PROD_MONTH);
}

void EltekRectifier::onProdDay(uint8_t d)
{
    spec.prodDay = d;
    if (EltekMgr::debugOn) log("PSU[%u]: prodDay: %u\n", info.id, spec.prodDay);
    requests.clear(ELTEK_PROD_DAY);
}
