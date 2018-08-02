#include "board_mgr.h"
#include "common.h"
#include "mother_board.h"
#include "board_revisions.h"
#include "format.hpp"
#include "data_noise_filter.h"
#include "i2c_board_access.h"
#include "polling_timer.h"
#include "master_gate.h"
#include "power_mgr.h"
#include "eltek_mgr.h"
#include "multy_board_mgr.h"
#include "pwrmodule_mgr.h"

/* lxj add begin 20180328 */
#include "ms_error.h"
/* lxj add end */

extern MultyBoardMgr g_multyBoardMgr;

BoardMgr::BoardMgr()
    : boardId(0),
      psLowTime(0),
      psLowNum(0),
      coolingStartTime(0),
	  heaterErrPrev(false)
{
    configureNone();
}

void BoardMgr::init(uint8_t id)
{
    boardId = id;

    // wait config from master to power up board
    config.powerOn = false;
}

void validateParam(const char* name, uint8_t &value, uint8_t maxValue)
{
    if (value > maxValue)
    {
        log("ERROR: param %s => %d: out of range, setting to %d\n",
            name, value, maxValue);

        value = maxValue;
    }
}

void BoardMgr::validateParams()
{
    validateParam("spiNum", spec.spiNum, MAX_SPI_PER_BOARD);
    validateParam("spiLen", spec.spiLen, MAX_PWC_PER_SPI);
    validateParam("btcNum", spec.btcNum, MAX_BTC16_PER_PWC);
}

void BoardMgr::autoConfigure()
{
    log("BOARD %d: auto detect...\n", boardId);
	uint32_t adcValue = 0;

    RevisionData *revisionData = nullptr;

    for (int i = 0; i < 5; i++)
    {
        g_motherBoard.loadBoardsAdc(); // TODO[improve]: load only one ADC
        adcValue = getBoardRevision();
        revisionData = BoardRevisions::findRevision(adcValue);

        log("Reading Rev, step %d: adc=%u => %s\n", i, adcValue, (revisionData ? "OK" : "no"));
        if (revisionData) break;
    }

    if (revisionData)
    {
        info.boardFound = true;
        spec = revisionData->spec;

        log("BOARD %d: found revision = %d\n", boardId, spec.revisionId);
    }
    else
    {
        log("BOARD %d: board not found!\n", boardId);
        configureNone();
    }

    validateParams();
}

void BoardMgr::configure(const BoardSpec &manualSpec)
{
    log("BOARD[%d]: manual spec: ", boardId);
    manualSpec.dump();

    info.boardFound = true;
    spec = manualSpec;

    validateParams();
}

void BoardMgr::configureNone()
{
    memset(&info, 0, sizeof(info));
    memset(&spec, 0, sizeof(spec));
}

void BoardMgr::updateInfo()
{
    info.revAdc = getBoardRevision();

    for (int i = 0; i < spec.getNumTmp(); i++)
    {
        info.boardTemperature[i] = getBoardTemp(i);
    }

    if (spec.isHeatSinkErrPin())
    {
        info.heaterErr = g_motherBoard.loadHeaterErrPin(boardId);
        if (heaterErrPrev != info.heaterErr)
        {
            log("BOARD[%d]: heatsink err %u => %u (n=%u)\n", boardId,
                heaterErrPrev, info.heaterErr, info.heaterErrNum);

            heaterErrPrev = info.heaterErr;
            info.heaterErrNum++;
        }
    }

    info.voltage = getBoardVoltage();

    for (int i = 0; i < spec.pwrNum; i++)
    {
        info.currents[i] = getBoardCurrent(i);
    }

	//chenbo add begin 20180123
	getBoardSNforHash();
	//chenbo add end

	//gezhihua add begin 20180423
	getBoardBinforHash();
	//gezhihua add end
}

uint32_t BoardMgr::getBoardRevision()
{
    uint16_t adc = g_motherBoard.getBoardsAdc(boardId, BOARD_ADC_LINE_REV);

    //log("getBoardRevision[%d]: adc=%d\n", boardId, adc);
    return adc;
}

uint8_t BoardMgr::getBoardTemp(uint8_t addr)
{
    I2CBoardAccess access(boardId);
    I2CTmp75 tmp75 = access.tmp75(addr);

    return tmp75.read() ? tmp75.temp : 0;
}

int8_t BoardMgr::getBoardMaxTemp(void) /* max temp between up/down temp */
{
	int8_t max = info.boardTemperature[0];
    for (int i = 0; i < spec.getNumTmp(); i++) {
		if(info.boardTemperature[i] > max) {
			max = info.boardTemperature[i];
		}
    }

	return max;
}

uint32_t BoardMgr::getBoardVoltage()
{
    uint16_t adc = g_motherBoard.getBoardsAdc(boardId, BOARD_ADC_LINE_U);
    int voltage = adc * spec.adcKu;

    if (0) {
        log("getBoardVoltage[%d]: adc=%d, U=%d mV\n", boardId, adc, voltage);
    }
    return voltage;
}

uint32_t BoardMgr::getBoardCurrent(int pwrLine)
{
    static const BoardAdcLine pwrLineToAdcLine[2] = {
        BOARD_ADC_LINE_I0, BOARD_ADC_LINE_I1
    };

    if (pwrLine > 2) {
        log("ERROR: unexpected pwrLine!\n");
        pwrLine = 0;
    }

    uint16_t adc = g_motherBoard.getBoardsAdc(boardId, pwrLineToAdcLine[pwrLine]);
    int ma = adc * spec.adcKi;

    if (0) {
        log("getBoardCurrent[%d/%d]: ADC=%d, I=%d mA\n", boardId, pwrLine, adc, ma);
    }
    return ma;
}

//chenbo add begin 20180108
void BoardMgr::setBoardOSCconfig(uint8_t OSC)
{
	I2CBoardAccess access(boardId);
    I2CNT3H1X01 nt3h1x01 = access.nt3h1x01(1);

	nt3h1x01.WriteOSCConfig(OSC);
}

void BoardMgr::getBoardOSCconfig()
{
	I2CBoardAccess access(boardId);
    I2CNT3H1X01 nt3h1x01 = access.nt3h1x01(1);

	info.boardOSC = nt3h1x01.ReadOSCConfig();
}
//chenbo add end

//chenbo add begin 20180122
void BoardMgr::setBoardSNforHash(char * sn_str)
{
	I2CBoardAccess access(boardId);
    I2CNT3H1X01 nt3h1x01 = access.nt3h1x01(1);

	nt3h1x01.WriteSNforHash(sn_str);
}

void BoardMgr::getBoardSNforHash()
{
	I2CBoardAccess access(boardId);
    I2CNT3H1X01 nt3h1x01 = access.nt3h1x01(1);

	memset(info.hashSN, 0, HASH_BOARD_SN_LENGHT);

	nt3h1x01.ReadSNforHash(info.hashSN);
}
//chenbo add end

//gezhihua add begin 20180423
void BoardMgr::setBoardBinforHash(uint8_t bin)
{
	I2CBoardAccess access(boardId);
    I2CNT3H1X01 nt3h1x01 = access.nt3h1x01(1);

	nt3h1x01.WriteBinConfig(bin);
}

void BoardMgr::getBoardBinforHash()
{
	I2CBoardAccess access(boardId);
    I2CNT3H1X01 nt3h1x01 = access.nt3h1x01(1);

	info.boardBin = nt3h1x01.ReadBinConfig();
}
//gezhihua add end

void BoardMgr::onBoardConfig(BoardConfig &newConfig)
{
    config = newConfig;
}

int BoardMgr::checkFanSpeedThreshold(uint8_t temp)
{
	if (temp <= HSB_TEMP_LOW) {
		return FAN_SPEED_BASE;
	} else {
		return (FAN_SPEED_BASE + ((temp - HSB_TEMP_LOW) * FAN_CTL_STEP));
	}
}

bool BoardMgr::checkBoardWorkCond(int *false_reason, int init_flag)
{
	uint32_t errCode;
	bool ret = true;
	*false_reason = 0;

	if(g_ModuleMgr.pHBWorkCond == nullptr){
		return false;
	}

	if (g_ModuleMgr.pHBWorkCond->HashTempFlag)
	{
		/* hash board check */
	    for (int i = 0; i < spec.getNumTmp(); i++)
		{
			if (init_flag)
			{
		        if (info.boardTemperature[i] > g_slaveCfg.maxTempLo)
				{
		            log("BOARD[%d]: temp[%d] %dC > %dC (maxTempLo): cooling mode\n",
		                boardId, i, info.boardTemperature[i], g_slaveCfg.maxTempLo);
					*false_reason = ERR_HB_TEMP;
					errCode = MS_SET_HW_ERR_CODE(ERR_HB_TEMP, ERR_TYPE_TMP_LOW, i);
					g_slaveError.saveErrorCode(errCode);
		            ret = false;
		        }
			}
			else
			{
				if (info.boardTemperature[i] > g_ModuleMgr.pHBWorkCond->HashTempHi)//g_slaveCfg.maxTempHi)
				{
					log("BOARD[%d]: temp[%d] %dC > %dC (maxTempHi): cooling mode\n",
						boardId, i, info.boardTemperature[i], g_ModuleMgr.pHBWorkCond->HashTempHi);//g_slaveCfg.maxTempHi);
					*false_reason = ERR_HB_TEMP;
					errCode = MS_SET_HW_ERR_CODE(ERR_HB_TEMP, ERR_TYPE_TMP_HIGH, i);
					g_slaveError.saveErrorCode(errCode);
					ret = false;
				}
			}
	    }
	}

	if (g_ModuleMgr.pHBWorkCond->PsuFlag)
	{
		if(g_ModuleMgr.pModuleMgr != nullptr)
		{
			if(!g_ModuleMgr.pModuleMgr->checkPowerCond(g_ModuleMgr.pHBWorkCond))
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_PWR, ERR_TYPE_PSU_TMP_HIGH, 0);
				g_slaveError.saveErrorCode(errCode);
				*false_reason = ERR_PWR;
				ret = false;
			}
		}
		else
		{
			errCode = MS_SET_SW_ERR_CODE(ERR_PWR, ERR_TYPE_PTR_NULL, 0);
			g_slaveError.saveErrorCode(errCode);
			*false_reason = ERR_PWR;
			ret = false;
		}
	}

	if (g_ModuleMgr.pHBWorkCond->FanFlag)
	{
		if((g_motherBoard.mbInfo.fan.getFanI() > 18) ||(g_motherBoard.mbInfo.fan.getFanU() > 14)) 
		{
			if (g_motherBoard.mbInfo.fan.getFanI() > 18)
			{				
				errCode = MS_SET_HW_ERR_CODE(ERR_FAN, ERR_TYPE_I_HIGH, 0);
				g_slaveError.saveErrorCode(errCode);
			}
			else
			{				
				errCode = MS_SET_HW_ERR_CODE(ERR_FAN, ERR_TYPE_U_HIGH, 0);
				g_slaveError.saveErrorCode(errCode);
			}
			*false_reason = ERR_FAN;
			ret = false;
		} 

		/* For unknown reason, the fault value may be zero when the fan stopped,
			so do the speed check here will keep machine more safe */
		uint32_t FanRpmLow = g_ModuleMgr.pHBWorkCond->FanWorkMode == FAN_FULL_SPEED?g_ModuleMgr.pHBWorkCond->FanRpmLowTh:2000;
		for(int fanId = 0; fanId < FAN_NUM; fanId++) 
		{
			if (g_masterGate.masterData.fanStates.fan_fault[fanId] > 0)
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_FAN, ERR_TYPE_FAN_FAULT, fanId);
				g_slaveError.saveErrorCode(errCode);
				*false_reason = ERR_FAN;
				ret = false;
			}

			if (g_masterGate.masterData.fanStates.RPM_Speed[fanId] < FanRpmLow)
			{
				errCode = MS_SET_HW_ERR_CODE(ERR_FAN, ERR_TYPE_SPEED_LOW, fanId);
				g_slaveError.saveErrorCode(errCode);				   
				*false_reason = ERR_FAN;
				ret = false;
			}
		}
	}
	return ret;
}

void BoardMgr::processPwrSw()
{
    uint32_t now = getMiliSeconds();
	int false_reason = 0;

    bool isOn = g_motherBoard.pwrSwitch[boardId].isOn;
    bool targetOn = false;

    if (!g_powerMgr.isPowerDetected()) {
        if (isOn) log("BOARD[%u]: off reason: no pwr", boardId);
    }
    else if (!info.boardFound) {
        if (isOn) log("BOARD[%u]: off reason: no brd", boardId);
    }
    else if (!config.powerOn) {
        if (isOn) log("BOARD[%u]: off reason: cfg brd off", boardId);
    }
    else if (info.heaterErr) {
        if (isOn) log("BOARD[%u]: off reason: heater err", boardId);
    }
    else if (g_masterGate.getPingTime() > 240*1000) {
		/* To decrease the risk of hash board power-down, 
		   enlarge the ping-time threshold, modify by gezhihua 20180309 */
        if (isOn) log("BOARD[%u]: off reason: no master", boardId);
    }
    else {
        targetOn = true;
    }

	/* ????????????????? */
    if (isOn != targetOn)
    {
        log("BOARD[%d]: changed targetOn: %u (PSU=%u F=%u ON=%u)\n", boardId,
            targetOn, g_powerMgr.isPowerDetected(), info.boardFound, config.powerOn);

        if (targetOn) {
            if (!checkBoardWorkCond(&false_reason, 1))
            {
                if (coolingStartTime == 0)
                {
                    // cooling mode: begin
                    coolingStartTime = now;
                    info.ohNum++;
                }
                else {
                    // cooling mode: in progress
                    info.ohTime = coolingTotalTime + (now - coolingStartTime);
                }
                return;
            }
        }

        log("BOARD[%u]: power %s", boardId, (targetOn ? "ON" : "OFF"));
        g_motherBoard.pwrSwitch[boardId].set(targetOn);
        return;
    }

	/* ??????????????????? */
    if (coolingStartTime > 0)
    {
        // cooling mode: end
        coolingTotalTime += (now - coolingStartTime);
        coolingStartTime = 0;

        info.ohTime = coolingTotalTime;
    }

    if (!targetOn) {
        return;
    }

	if(!checkBoardWorkCond(&false_reason, 0)){
		info.ohNum++;
		g_motherBoard.pwrSwitch[boardId].set(false);
		coolingStartTime = now;
		log("board %d poweroff by reason: 0x%x\n", boardId, false_reason);
		return;
	}
	
    bool monitorCurrents = (g_slaveCfg.psLowCurrent > 0);

    if (spec.isMissingCurrent() && spec.isTypeB()) {
        monitorCurrents = false;
    }

    if (monitorCurrents)
    {
        // monitoring current low threshold
        bool currentOk = true;
        for (int i = 0; i < spec.pwrNum; i++)
        {
            if (info.currents[i] < g_slaveCfg.psLowCurrent)
            {
                currentOk = false; break;
            }
        }

        if (currentOk) {
            psLowTime = now;
            psLowNum = 0;
        }
        else {
            psLowNum++;

            if ((now - psLowTime > g_slaveCfg.psLowCurrentPeriod * 1000) && (psLowNum > 10))
            {
                log("BOARD[%d]: low current detected, restart power switch!\n", boardId);

                log("BOARD[%u]: power ON (LoI)", boardId);
                g_motherBoard.pwrSwitch[boardId].set(true);

                psLowTime = now;
                psLowNum = 0;

                info.lowCurrRst++;
            }
        }
    }
}

void BoardMgr::storePwrSwTest(int index)
{
    BoardPwrSwTest &pst = test.pwrSwTest.items[index];

    pst.pwrOn = g_motherBoard.pwrSwitch[boardId].isOn;
    pst.voltage = info.voltage;

    for (int i = 0; i < spec.pwrNum; i++)
    {
        pst.currents[i] = info.currents[i];
    }
}

bool BoardMgr::writeTmpAlert(uint8_t tmpId, uint8_t value)
{
	/* g_slaveCfg will recover by master message,so set alert by g_ModuleMgr */
    uint8_t alertLo = g_ModuleMgr.pHBWorkCond->HashTempHi;//g_slaveCfg.maxTempHi;
    uint8_t alertHi = g_ModuleMgr.pHBWorkCond->HashTempHi;//g_slaveCfg.maxTempHi;

    if (value != 0xff) {
        alertLo = value;
        alertHi = value;
    }
    I2CBoardAccess boardI2C(boardId);
    I2CTmp75 tmp75 = boardI2C.tmp75(tmpId);

    // read/write alert lo
    bool okLo = tmp75.readAlertLo();

    if (okLo) {
        info.taInfo[tmpId].alertLo = tmp75.alertLo;
    }

    if (!okLo || tmp75.alertLo != alertLo)
    {
        info.taInfo[tmpId].numWrite++;

        if (!tmp75.writeAlertLo(alertLo)) {
            okLo = false;
        }
    }

    // read/write alert hi
    bool okHi = tmp75.readAlertHi();

    if (okHi) {
        info.taInfo[tmpId].alertHi = tmp75.alertHi;
    }

    if (!okHi || tmp75.alertHi != alertHi)
    {
        info.taInfo[tmpId].numWrite++;

        if (!tmp75.writeAlertHi(alertHi)) {
            okHi = false;
        }
    }

    return okLo && okHi;
}

void BoardMgr::dump()
{
    log("BOARD[%d] info: ", boardId); info.dump();
    log("BOARD[%d] spec: ", boardId); spec.dump();
}
