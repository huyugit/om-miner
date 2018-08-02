#ifndef DATA_FEILD_H
#define DATA_FEILD_H


#if defined(POWER_SUPPLY_USE_HUAWEI_R48XX)

#include <stdint.h>

class dataFeild
{
public:
	dataFeild();
	dataFeild(uint16_t sId);

	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1){return addr + data0 + data1;};

	uint16_t getSingalId(void);
	
private:
	uint16_t	sigId;
};

//signal ID:	0x0170
class inputPowerFeild : public dataFeild
{
public:
	inputPowerFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0171
class inputFreqFeild : public dataFeild
{
public:
	inputFreqFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0172
class inputCurrentFeild : public dataFeild
{
public:
	inputCurrentFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0173
class outPowerFeild : public dataFeild
{
public:
	outPowerFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0174
class rtEfficiencyFeild : public dataFeild
{
public:
	rtEfficiencyFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0175
class outputVoltageFeild : public dataFeild
{
public:
	outputVoltageFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0180
class inputTempFeild : public dataFeild
{
public:
	inputTempFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

//signal ID:	0x0181
class outputTempFeild : public dataFeild
{
public:
	outputTempFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};


//signal ID:	0x0182  shu chu dian liu
class outputCurrentFeild : public dataFeild
{
public:
	outputCurrentFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};


//signal ID:	0x0001 
class characterFeild : public dataFeild
{
public:
	characterFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};


//signal ID:	0x0005  
class versionFeild : public dataFeild
{
public:
	versionFeild();
	virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};

       //signal ID:    0x0178
class inputVoltageFeild : public dataFeild
{
public:
       inputVoltageFeild();
       virtual uint32_t parseDataFeild(uint32_t addr, uint32_t data0, uint32_t data1);
};


#endif

#endif
