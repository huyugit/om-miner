/*******************************************************************************
*  file    : eltekrxmsg.cpp
*  created : 20.03.2015
*  author  : Slyshyk Oleksiy
*******************************************************************************/

#include <cstring>
#include "eltek_msg.h"
#include "eltek_mgr.h"
#include "format.hpp"
#include "mytime.h"


uint32_t EltekRxMsg::systemType(const CanRxMsg &msg)
{
    uint32_t res = 0;
    res = (msg.ExtId >> 24) & 0x0000001f;
    return res;
}

uint32_t EltekRxMsg::systemIndex(const CanRxMsg &msg)
{
    uint32_t res = 0;
    res = (msg.ExtId >> 16) & 0x000000ff;
    return res;
}

uint32_t EltekRxMsg::msgType(const CanRxMsg &msg)
{
    uint32_t res = 0;
    res = (msg.ExtId >> 14) & 0x00000003;
    return res;
}

uint32_t EltekRxMsg::variant(const CanRxMsg &msg)
{
    uint32_t res = 0;
    res = (msg.ExtId >> 10) & 0x0000000f;
    return res;
}



EltekLogOnMsg::EltekLogOnMsg(const CanRxMsg &msg)
{
    systemType_  = EltekRxMsg::systemType(msg);
    systemIndex_ = EltekRxMsg::systemIndex(msg);
    msgType_     = EltekRxMsg::msgType(msg);
    variant_     = EltekRxMsg::variant(msg);

    serial_ = 0;
    for (int i = 0 ; i < 6; ++i)
    {
        ((uint8_t*)(&serial_))[i] = msg.Data[5 - i] ;
    }

    features_ = 0;
    ((uint8_t*)(&features_))[0] = msg.Data[6];
    ((uint8_t*)(&features_))[1] = msg.Data[7];
}

CanTxMsg EltekLogOnMsg::logOnId(uint8_t ID) const
{
    CanTxMsg msg;
    ::memset(&msg,0,sizeof(msg));
    msg.IDE = CAN_Id_Extended;
    msg.RTR = CAN_RTR_Data;

    msg.ExtId |= (5  << 24) ;//System type
    msg.ExtId |= (1  << 14) ;//Message type
    msg.ExtId |= (2  << 10) ;//Variant
    msg.ExtId |= (ID << 2 ) ;//New ID

    for (int i = 0; i < 6; ++i)
    {
        msg.Data[i] = ((uint8_t*)(&serial_))[5 - i] ;
    }

    msg.DLC = 8;
    return msg;
}


EltekStatusMsg::EltekStatusMsg(const CanRxMsg &msg)
{
//    Data 0    0-255    : Condition
//    Data 1    0-255    : In air temperature
//    Data 2    0-255    : Measured current LSB
//    Data 3    0-255    : Measured current MSB
//    Data 4    0-255    : Measured Voltage LSB
//    Data 5    0-255    : Measured Voltage MSB
//    Data 6    0-255    : Measured mains voltage LSB
//    Data 7    0-255    : Measured mains voltage MSB
//    Data 8    0-255    : Out air temperature


    systemType_  = EltekRxMsg::systemType(msg);
    systemIndex_ = EltekRxMsg::systemIndex(msg);
    msgType_     = EltekRxMsg::msgType(msg);
    variant_     = EltekRxMsg::variant(msg);

    condition_ = (msg.ExtId >> 2) ;
    in_air_temperature_                       = msg.Data[0];
    ((uint8_t*)(&measured_current_))[0]       = msg.Data[1];
    ((uint8_t*)(&measured_current_))[1]       = msg.Data[2];
    ((uint8_t*)(&measured_voltage_))[0]       = msg.Data[3];
    ((uint8_t*)(&measured_voltage_))[1]       = msg.Data[4];
    ((uint8_t*)(&measured_mains_voltage_))[0] = msg.Data[5];
    ((uint8_t*)(&measured_mains_voltage_))[1] = msg.Data[6];
    out_air_temperature_                      = msg.Data[7];	// TODO Data[7] is uint8_t
}

const char *EltekStatusMsg::condition_to_string(uint8_t cnd)
{
    switch (cnd)
    {
    case COND_Error:        return "Error";
    case COND_Normal:       return "Normal";
    case COND_Minor_Alarm:  return "Minor Alarm";
    case COND_Major_Alarm:  return "Major Alarm";
    case COND_Disabled:     return "Disabled";
    case COND_Disconnected: return "Disconnected";
    case COND_Major_Low:    return "Major Low";
    case COND_Minor_Low:    return "Minor Low";
    case COND_Major_High:   return "Major High";
    case COND_Minor_High:   return "Minor High";
    }
    return "UNKNOWN";
}


uint16_t EltekControlSystemStatusMsg::currentReference() const
{
    return current_reference_;
}

void EltekControlSystemStatusMsg::setCurrentReference(const uint16_t &current_reference)
{
    if(current_reference > (99 * 10))
        current_reference_ = 0xffff;
    else
        current_reference_ = current_reference;
}
uint16_t EltekControlSystemStatusMsg::measuredOutputVoltage() const
{
    return measured_output_voltage_;
}

void EltekControlSystemStatusMsg::setMeasuredOutputVoltage(const uint16_t &measured_output_voltage)
{
    measured_output_voltage_ = measured_output_voltage;
}
uint16_t EltekControlSystemStatusMsg::outputVoltageReference() const
{
    return output_voltage_reference_;
}

void EltekControlSystemStatusMsg::setOutputVoltageReference(const uint16_t &output_voltage_reference)
{
    output_voltage_reference_ = output_voltage_reference;
}
uint16_t EltekControlSystemStatusMsg::OVP() const
{
    return OVP_;
}

void EltekControlSystemStatusMsg::setOVP(const uint16_t &OVP)
{
    OVP_ = OVP;
}
bool EltekControlSystemStatusMsg::quickWalkIn() const
{
    return quick_walk_in_;
}

void EltekControlSystemStatusMsg::setQuickWalkIn(bool quick_walk_un)
{
    quick_walk_in_ = quick_walk_un;
}

CanTxMsg EltekControlSystemStatusMsg::canMessage()
{
    CanTxMsg msg;
    ::memset(&msg,0,sizeof(msg));
    msg.IDE = CAN_Id_Extended;
    msg.RTR = CAN_RTR_Data;

    msg.ExtId |= (5   << 24) ;//System type
    msg.ExtId |= (1   << 14) ;//Message type
    msg.ExtId |= (system_index_ << 16) ;//System Index

    msg.ExtId |= (1   << 2 ) ;//Condition
    if(quick_walk_in_ == false)
        {
            msg.ExtId |= 1 ;//slow walk in;
        }

    msg.Data[0] = ((uint8_t*)(&current_reference_))[0];
    msg.Data[1] = ((uint8_t*)(&current_reference_))[1];
    msg.Data[2] = ((uint8_t*)(&measured_output_voltage_))[0];
    msg.Data[3] = ((uint8_t*)(&measured_output_voltage_))[1];
    msg.Data[4] = ((uint8_t*)(&output_voltage_reference_))[0];
    msg.Data[5] = ((uint8_t*)(&output_voltage_reference_))[1];
    msg.Data[6] = ((uint8_t*)(&OVP_))[0];
    msg.Data[7] = ((uint8_t*)(&OVP_))[1];

    msg.DLC = 8;
    return msg;
}
uint8_t EltekControlSystemStatusMsg::systemIndex() const
{
    return system_index_;
}

void EltekControlSystemStatusMsg::setSystemIndex(const uint8_t &system_index)
{
    system_index_ = system_index;
}



EltekGenMessage::EltekGenMessage()
    : systemIndex(0),
      subSystem(0),
      subSystemIndex(0),
      dataObject(0),
      dataElement(0),
      messageIndex(0),
      dataSize(0),
      lastMessageIndex(0),
      isComplete(false)
{
}

EltekGenMessage::EltekGenMessage(uint8_t _subSystem, uint8_t _subSystemIndex,
                                 uint8_t _dataObject, uint8_t _dataElement)
    : EltekGenMessage()
{
    subSystem = _subSystem;
    subSystemIndex = _subSystemIndex;
    dataObject = _dataObject;
    dataElement = _dataElement;
}


EltekGenMessage::EltekGenMessage(const CanRxMsg &msg)
{
    systemIndex    = EltekRxMsg::systemIndex(msg);

    subSystem        = EltekRxMsg::variant(msg);
    subSystemIndex   = msg.ExtId >> 2;

    function = 0;
    function |= (msg.ExtId & 0x03) << 3 ;
    function |= msg.Data[0] & 0x07;

    dataObject = 0;
    dataObject |= msg.Data[0] >> 3;
    dataObject |= msg.Data[1] << 5;
    dataObject &= (~0x80);

    dataElement = 0;
    dataElement |= msg.Data[1] >> 2;

    messageIndex = msg.Data[2] & 0x7F;
    isStartOfFrame = msg.Data[2] & 0x80;

    for(uint32_t i = 0; i < 5; ++i)
        data[i] = msg.Data[i+3];
}

bool EltekGenMessage::isResponeTo(const EltekGenMessage &msg) const
{
    return
        subSystem         == msg.subSystem          &&
        subSystemIndex    == msg.subSystemIndex     &&
        dataObject        == msg.dataObject         &&
        dataElement       == msg.dataElement;
}

bool EltekGenMessage::addMessage(const EltekGenMessage& msg)
{
//    log("%6u: addMessage: systemIndex = %u, messageIndex = %u, start = %u\n",
//        getMiliSeconds(), msg.systemIndex, msg.messageIndex, msg.isStartOfFrame);

    if (msg.isStartOfFrame)
    {
        systemIndex     = msg.systemIndex;
        subSystem       = msg.subSystem;
        subSystemIndex  = msg.subSystemIndex;
        function        = msg.function;
        dataObject      = msg.dataObject;
        dataElement     = msg.dataElement;

        dataSize = 0;
        isComplete = false;
    }
    else
    {
        bool res = true;

        if( systemIndex     != msg.systemIndex)    res = false;
        if( subSystem       != msg.subSystem)      res = false;
        if( subSystemIndex  != msg.subSystemIndex) res = false;
        if( function        != msg.function)       res = false;
        if( dataObject      != msg.dataObject)     res = false;
        if( dataElement     != msg.dataElement)    res = false;

        if (!res)
        {
            if (EltekMgr::debugOn)
            {
                log("addMessage: FRAME SEQ ERROR: wrong system!\n");
                log("   current:  "); dumpAddr();
                log("   received: "); msg.dumpAddr();
            }
            return false;
        }

        if (msg.messageIndex != lastMessageIndex - 1)
        {
            if (EltekMgr::debugOn)
            {
                log("addMessage: FRAME SEQ ERROR: seq id mismatch: rx %u, last %u!\n",
                    msg.messageIndex, lastMessageIndex);
            }
            return false;
        }
    }

    for (int i = 0; i < 5; ++i)
        data[dataSize++] = msg.data[i];

    lastMessageIndex = msg.messageIndex;

    if (msg.messageIndex == 1)
    {
        isComplete = true;
    }

    return true;
}

CanTxMsg EltekGenMessage::canTx() const
{
    CanTxMsg msg;
    ::memset(&msg,0,sizeof(msg));
    msg.IDE = CAN_Id_Extended;
    msg.RTR = CAN_RTR_Data;
    msg.DLC = 3+dataSize;

    msg.ExtId |= (TX_SYSTEM_TYPE   << 24) ;//System type
    msg.ExtId |= (systemIndex  << 16) ;//System Index
    msg.ExtId |= (TX_MESSAGE_TYPE  << 14) ;//Message type
    msg.ExtId |= (subSystem      << 10) ;//SubSystem
    msg.ExtId |= (subSystemIndex << 2);
    msg.ExtId |= (function >> 3);

    msg.Data[0] = (dataObject << 3)  | (function & 0x07) ;
    msg.Data[1] = (dataElement << 2) | ((dataObject >> 5) & 0x03);
    msg.Data[2] = messageIndex;

    for(uint32_t i = 0; i < 5; ++i)
        msg.Data[i+3] = data[i];

    return msg;
}

void EltekGenMessage::dumpAddr() const
{
    log("SI=%u, SS=%u, SSI=%u, F=%u, DO=%u, DE=%u, MI=%d\n",
        systemIndex, subSystem, subSystemIndex, function, dataObject, dataElement, messageIndex);
}

void EltekGenMessage::setSystemIndex(const uint8_t &system_index)
{
    systemIndex = system_index & 0x0f;
}

void EltekGenMessage::setFunction(const uint8_t &value)
{
    function = value & 0x1f;
}

void EltekGenMessage::setData(const uint8_t *pd, uint32_t size)
{
    if (size > MAX_DATA_SIZE)
    {
        size = MAX_DATA_SIZE;
    }

    dataSize = size;

    for(uint32_t i = 0; i < size; ++i)
        data[i] = pd[i];
}

void EltekGenMessage::setDataObject(const uint8_t &x)
{
    dataObject = x & (~0x80);
}

void EltekGenMessage::setDataElement(const uint8_t &data_element)
{
    dataElement = data_element;
}
