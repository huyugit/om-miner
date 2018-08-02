/*******************************************************************************
*  file    : eltekrxmsg.hpp
*  created : 20.03.2015
*  author  : Slyshyk Oleksiy
*******************************************************************************/

#ifndef ELTEK_MSG_H
#define ELTEK_MSG_H

#include <array>
#include <cstdint>

#include "stm32f4xx_can.h"


enum EltekMessageType
{
    EMT_Current_Share       = 0,
    EMT_Status_LogOn        = 1,
    EMT_GenMessage          = 2,
    EMT_Software_Download   = 3
};

enum EltekStatusLogOnVariants
{
    ESV_Status         = 0,
    ESV_Login_Request  = 1,
    ESV_Login_responce = 2
};


class EltekRxMsg
{
public:
    static uint32_t systemType (const CanRxMsg& msg);
    static uint32_t systemIndex(const CanRxMsg& msg);
    static uint32_t msgType    (const CanRxMsg& msg);
    static uint32_t variant    (const CanRxMsg& msg);
};

class EltekLogOnMsg
{
public:
    EltekLogOnMsg(const CanRxMsg& msg);
    uint32_t systemType () const {return systemType_;}
    uint32_t systemIndex() const {return systemIndex_;}
    uint32_t msgType    () const {return msgType_;}
    uint32_t variant    () const {return variant_;}
    uint64_t serial     () const {return serial_;}

    CanTxMsg logOnId    (uint8_t ID) const;
private:
    uint32_t systemType_ ;
    uint32_t systemIndex_;
    uint32_t msgType_    ;
    uint32_t variant_    ;
    uint64_t serial_     ;

    uint16_t features_   ;
};

class EltekStatusMsg
{
public:
    EltekStatusMsg(const CanRxMsg& msg);

    uint32_t systemType () const {return systemType_;}
    uint32_t systemIndex() const {return systemIndex_;}
    uint32_t msgType    () const {return msgType_;}
    uint32_t variant    () const {return variant_;}

    uint8_t  condition             () const {return condition_;}
    int8_t  in_air_temperature    () const {return in_air_temperature_;}
    uint16_t measured_current      () const {return measured_current_;}
    uint16_t measured_voltage      () const {return measured_voltage_;}
    uint16_t measured_mains_voltage() const {return measured_mains_voltage_;}
    int8_t  out_air_temperature   () const {return out_air_temperature_;}

    static const char *condition_to_string(uint8_t cnd);
private:
    uint32_t systemType_ ;
    uint32_t systemIndex_;
    uint32_t msgType_    ;
    uint32_t variant_    ;

    uint8_t  condition_;
    int8_t  in_air_temperature_;
    uint16_t measured_current_;
    uint16_t measured_voltage_;
    uint16_t measured_mains_voltage_;
    int8_t  out_air_temperature_;
};

class EltekControlSystemStatusMsg
{
public:
    EltekControlSystemStatusMsg() {}

    uint16_t    currentReference() const;
    void        setCurrentReference(const uint16_t &currentReference);

    uint16_t    measuredOutputVoltage() const;
    void        setMeasuredOutputVoltage(const uint16_t &measuredOutputVoltage);

    uint16_t    outputVoltageReference() const;
    void        setOutputVoltageReference(const uint16_t &outputVoltageReference);

    uint16_t    OVP() const;
    void        setOVP(const uint16_t &OVP);

    bool        quickWalkIn() const;
    void        setQuickWalkIn(bool quickWalkIn);

    CanTxMsg    canMessage();

    uint8_t     systemIndex() const;
    void        setSystemIndex(const uint8_t &systemIndex);

private:
    uint8_t  system_index_              = 255;      // Broadcast
    uint8_t  condition_                 = 1;
    uint16_t current_reference_         = 0xFFFF;   // deactivated
    uint16_t measured_output_voltage_   = 0;
    uint16_t output_voltage_reference_  = 0;
    uint16_t OVP_                       = 5950;     // 59.5V output voltage protection
    bool     quick_walk_in_             = true;
};

class EltekGenMessage
{
public:
    EltekGenMessage();

    EltekGenMessage(uint8_t _subSystem, uint8_t _subSystemIndex,
                    uint8_t _dataObject, uint8_t _dataElement);

    EltekGenMessage(const CanRxMsg &msg);

    bool isResponeTo(const EltekGenMessage &msg) const;


    // FRAME handling functions
    bool addMessage(const EltekGenMessage& msg);

    void setData(const uint8_t* pd, uint32_t size);

    void    setDataObject       (const uint8_t &x);
    void    setDataElement      (const uint8_t &dataElement);
    void    setSystemIndex      (const uint8_t &systemIndex);
    void    setFunction         (const uint8_t &value);

    CanTxMsg canTx() const;
    void dumpAddr() const;


    static const uint8_t TX_SYSTEM_TYPE     = 5;
    static const uint8_t TX_MESSAGE_TYPE    = 2;
    static const uint8_t MAX_DATA_SIZE      = 30;

    uint8_t systemIndex;
    uint8_t subSystem;
    uint8_t subSystemIndex;
    uint8_t function;
    uint8_t dataObject;
    uint8_t dataElement;

    bool    isStartOfFrame;
    uint8_t messageIndex;

    uint8_t dataSize;
    uint8_t data[MAX_DATA_SIZE];

    uint32_t                lastMessageIndex;
    bool                    isComplete;
};

#endif // ELTEK_MSG_H
