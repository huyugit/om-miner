#include "JsonStat.h"

#include <cstdio>
#include <time.h>

#include "format.hpp"
#include "version.h"
#include "app/Application.h"
#include "ms-protocol/ms_packet.h"
#include "pool/StratumPool.h"
#include "env/EnvManager.h"
#include "hw/GpioManager.h"
#include "stats/MasterStat.h"
#include "stats/hist/HistoryStatCalc.h"
#include "base/DateTimeStr.h"
#include "sys/NetInfoUtil.h"


class JsonGenerator
{
public:
    JsonGenerator(FILE *_fd)
        : fd(_fd), firstLine(true), separatorRequired(false), indentLevel(0)
    {}

    void beginObject() {
        newLine(); fprintf(fd, "{");
        separatorRequired = false; indentLevel++;
    }

    void beginObject(const char *name) {
        newLine(); fprintf(fd, "\"%s\": {", name);
        separatorRequired = false; indentLevel++;
    }

    void endObject() {
        separatorRequired = false; indentLevel--;
        newLine(); fprintf(fd, "}");
        separatorRequired = true;
    }

    void addNameValue(const char *name, int value) {
        newLine();
        fprintf(fd, "\"%s\": %d", name, value);
    }

    void addNameValue(const char *name, unsigned int value) {
        newLine();
        fprintf(fd, "\"%s\": %u", name, value);
    }

    void addNameValue(const char *name, int64_t value) {
        addNameValueF(name, value, "%lld");
    }

    void addNameValue(const char *name, uint64_t value) {
        addNameValueF(name, value, "%llu");
    }

    void addNameValue(const char *name, double value, const char *format="%f") {
        addNameValueF(name, value, format);
    }

    void addNameValue(const char *name, const char *value) {
        newLine();
        fprintf(fd, "\"%s\": \"%s\"", name, value);
    }

    template<typename T>
    void addNameValueF(const char *name, const T &value, const char *format) {
        static char buffer[128];
        snprintf(buffer, sizeof(buffer), format, value);

        newLine();
        fprintf(fd, "\"%s\": \"%s\"", name, buffer);
    }

    void beginArray() {
        newLine(); fprintf(fd, "[");
        separatorRequired = false; indentLevel++;
    }

    void beginArray(const char *name) {
        newLine(); fprintf(fd, "\"%s\": [", name);
        separatorRequired = false; indentLevel++;
    }

    void endArray() {
        separatorRequired = false; indentLevel--;
        newLine(); fprintf(fd, "]");
        separatorRequired = true;
    }

    void addValue(int value) {
        newLine();
        fprintf(fd, "%d", value);
    }

    void addValue(unsigned int value) {
        newLine();
        fprintf(fd, "%u", value);
    }

    void addValue(double value, const char *format="%f") {
        static char buffer[128];
        snprintf(buffer, sizeof(buffer), format, value);

        newLine();
        fprintf(fd, "%s", buffer);
    }

    void addValue(const char *value) {
        newLine();
        fprintf(fd, "\"%s\"", value);
    }

private:
    FILE *fd;
    bool firstLine, separatorRequired;
    int indentLevel;

    void newLine() {
        if (firstLine) {
            firstLine = false;
            return;
        }

        if (separatorRequired) {
            fprintf(fd, ",");
        }

        fprintf(fd, "\n");

        for (int i = 0; i < indentLevel; i++) {
            fprintf(fd, "\t");
        }

        separatorRequired = true;
    }
};

void JsonStat::exportStat()
{
    printf("Exporting stat... ");

    const char* statFilename = "/run/sm-miner.stat";

    FILE *fd = fopen(statFilename, "w");
    if (fd != nullptr)
    {
        JsonGenerator gen(fd);
        char buffer[128];

        StratumPool& pool = Application::pool();

        gen.beginObject();
        {
            gen.addNameValue("date", DateTimeStr().str);
            gen.addNameValue("statVersion", "1.2");

            gen.beginObject("versions");
            {
                gen.addNameValue("miner",       swVersion);
                gen.addNameValue("minerDate",   swVersionDate);
                gen.addNameValue("minerDebug",  "0");

                snprintf(buffer, sizeof(buffer), "0x%04x", MsPacket::VERSION);
                gen.addNameValue("mspVer",      buffer);
            }
            gen.endObject();

            gen.beginObject("master");
            {
                gen.addNameValue("upTime",      g_masterStat.statIntervalTotal);
                gen.addNameValue("mbHwVer",     g_gpioManager.mbHwVer);
                gen.addNameValue("diff",        Application::pool().getCurrentDifficulty());
                gen.addNameValue("boards",      g_masterStat.total.boards);
                gen.addNameValue("errorSpi",    g_masterStat.total.numBrokenSpi);
                gen.addNameValue("osc",         Application::config()->pwcConfig.osc);

                gen.addNameValue("ipWhite",     util::ipAddressToString(util::getIpAddress("eth0")).cdata());
                gen.addNameValue("ipGray",      util::ipAddressToString(util::getIpAddress("tap0")).cdata());
                gen.addNameValue("hwAddr",      util::macAddressToString(util::getMacAddress("eth0")).cdata());

                HistoryStatCalc calc(HistoryStatMgr::PERIOD_TOTAL);
                double wghs = util::safeDiv(g_masterStat.total.psuPower, calc.getGHs());

                gen.addNameValue("voltSet",     Application::configRW().psuConfig.getVoltage(), "%.1f");
                gen.addNameValue("psuI",        g_masterStat.total.psuCurrent, "%.1f");
                gen.addNameValue("psuW",        g_masterStat.total.psuPower,   "%.1f");
                gen.addNameValue("boardsI",     (double)g_masterStat.total.currentA / 1000, "%.1f");
                gen.addNameValue("boardsW",     (double)g_masterStat.total.power, "%.1f");
                gen.addNameValue("wattPerGHs",  wghs, "%.3f");


                gen.beginObject("intervals");
                {
                    for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
                    {
                        HistoryStatCalc calc(period);

                        snprintf(buffer, sizeof(buffer), "%d", calc.getStat().intervalExpected);
                        gen.beginObject(buffer);
                        {
                            gen.addNameValue("name",            calc.getStat().intervalName);
                            gen.addNameValue("interval",        calc.getSeconds());

                            gen.addNameValue("bySol",           calc.getGHs(),         "%.1f");
                            gen.addNameValue("byDiff",          calc.getGHsByDiff(),   "%.1f");
                            gen.addNameValue("byPool",          calc.getGHsByPool(),   "%.1f");
                            gen.addNameValue("byJobs",          calc.getGHsByJobs(),   "%.1f");

                            gen.addNameValue("solutions",       calc.getSol());
                            gen.addNameValue("errors",          calc.getErr());
                            gen.addNameValue("errorRate",       calc.getErrToSol(),    "%.1f");
                            gen.addNameValue("chipSpeed",       calc.getChipGHs(),     "%.2f");
                            gen.addNameValue("chipRestarts",    calc.getChipRestarts());
                        }
                        gen.endObject();
                    }
                }
                gen.endObject();
            }
            gen.endObject();

            gen.beginObject("pool");
            {
		// fcj modify begin 20180314
		#ifdef MULTI_POOL_SUPPORT
				gen.addNameValue("host", pool.getCurrentPoolInfo().host.cdata());
				gen.addNameValue("port", pool.getCurrentPoolInfo().port.cdata());
				gen.addNameValue("userName", pool.getCurrentPoolInfo().userName.cdata());
        #else
				gen.addNameValue("host", pool.getPoolConfig().host.cdata());
                gen.addNameValue("port", pool.getPoolConfig().port.cdata());
                gen.addNameValue("userName", pool.getPoolConfig().userName.cdata());
		#endif
		// fcj modify end

                gen.addNameValue("diff", pool.getCurrentDifficulty());

                gen.beginObject("intervals");
                {
                    for (size_t period = 0; period < HistoryStatMgr::MAX_PERIODS; period++)
                    {
                        const HistoryStat& periodStat = g_masterStat.getHistory().getByPeriod(period);

                        snprintf(buffer, sizeof(buffer), "%d", periodStat.intervalExpected);
                        gen.beginObject(buffer);
                        {
                            gen.addNameValue("name",                    periodStat.intervalName);
                            gen.addNameValue("interval",                periodStat.getSeconds());

                            gen.addNameValue("jobs",                    periodStat.poolReceivedJobs);
                            gen.addNameValue("cleanFlags",              periodStat.poolReceivedJobsWithClean);

                            gen.addNameValue("sharesSent",              periodStat.poolSentShares);
                            gen.addNameValue("sharesAccepted",          periodStat.poolAcceptedShares);
                            gen.addNameValue("sharesRejected",          periodStat.poolRejectedShares);
                            gen.addNameValue("solutionsAccepted",       periodStat.poolAcceptedSolutions);
                            gen.addNameValue("shareLoss",               periodStat.getShareLoss());

                            gen.addNameValue("poolTotal",               static_cast<uint32_t>(periodStat.poolTotalTime / 1000));
                            gen.addNameValue("inService",               static_cast<uint32_t>(periodStat.poolInService / 1000));

                            gen.addNameValue("subscribeError",          periodStat.poolSubscribeError);
                            gen.addNameValue("diffChanges",             periodStat.poolDiffChanges);
                            gen.addNameValue("reconnections",           periodStat.poolReconnections);
                            gen.addNameValue("reconnectionsOnErrors",   periodStat.poolReconnectionsOnError);

                            gen.addNameValue("defaultJobShares",        periodStat.poolDefaultJobShares);
                            gen.addNameValue("staleJobShares",          periodStat.poolStaleJobShares);
                            gen.addNameValue("duplicateShares",         periodStat.poolDuplicateShares);
                            gen.addNameValue("lowDifficultyShares",     periodStat.poolLowDifficultyShares);

                            gen.addNameValue("pwcSharesSent",           periodStat.pwcSharesSent);
                            gen.addNameValue("pwcSharesDropped",        periodStat.pwcSharesDropped);
                        }
                        gen.endObject();
                    }
                }
                gen.endObject();
            }
            gen.endObject();

            gen.beginObject("pwr");
            {
                PsuMgrInfo &pmi = g_masterStat.getSlave(0).psuMgrInfo;
                gen.addNameValue ("powerOn",    pmi.powerOn);
                gen.addNameValue ("setVoltage", pmi.setVoltage/100.0);
                gen.addNameValue ("stateId",    pmi.state);
                gen.addNameValue ("state",      powerStateToStr(pmi.getState()));
                gen.addNameValue ("stateMs",    pmi.stateSec);

            }
            gen.endObject();

            gen.beginArray("psu");
            {
                for (size_t i = 0; i < MAX_PSU_PER_SLAVE; i++)
                {
                    PsuInfo &pi = g_masterStat.getSlave(0).psuInfo[i];
                    PsuSpec &ps = g_masterStat.getSlave(0).psuSpec[i];
                    gen.beginObject();
                    {
                        gen.addNameValue ("id",         pi.id);
                        gen.addNameValue ("serial",     pi.serial);

                        gen.addNameValueF("vOut",       pi.getVOut(),   "%.1f");
                        gen.addNameValueF("iOut",       pi.getIOut(),   "%.1f");
                        gen.addNameValueF("pOut",       pi.getPOut(),   "%.0f");
                        gen.addNameValueF("vIn",        pi.getVIn(),    "%.1f");

                        gen.addNameValue ("cond",       pi.condition);
                        gen.addNameValue ("tIn",        pi.tempIn);
                        gen.addNameValue ("tOut",       pi.tempOut);

                        gen.addNameValue ("numSt",      pi.numStatus);
                        gen.addNameValue ("lstStT",     pi.lastStatusTime);

                        gen.addNameValue ("minAlarm",   pi.minorAlarm);
                        gen.addNameValue ("majAlarm",   pi.majorAlarm);

                        gen.addNameValue ("defV",       pi.defaultVoltage);

                        gen.addNameValue ("ledG",       pi.greenLed);
                        gen.addNameValue ("ledY",       pi.yellowLed);
                        gen.addNameValue ("ledR",       pi.redLed);

                        gen.addNameValue ("ut",         pi.upTime);
                        gen.addNameValue ("fsr",        pi.fanSpeedRef);
                        gen.addNameValue ("fs",         pi.fanSpeed);

                        gen.beginObject("prod");
                        {
                            gen.addNameValue("part",        ps.prodPart);
                            gen.addNameValue("ver",         ps.prodVer);

                            snprintf(buffer, sizeof(buffer), "%04u-%02u-%02u", ps.prodYear, ps.prodMonth, ps.prodDay);
                            gen.addNameValue("date",        buffer);

                            gen.addNameValue("desc",        ps.prodDesc);
                        }
                        gen.endObject();
                    }
                    gen.endObject();
                }
            }
            gen.endArray();

            FanConfig &fanConfig = Application::configRW().slaveConfig.fanConfig;
            FanInfo &fanInfo = g_masterStat.getSlave(0).cmnInfo.mbInfo.fan;

            gen.beginObject("fan");
            {
                gen.addNameValue("cfgU",    fanConfig.getFanVoltage(),  "%.3f");
                gen.addNameValue("fanU",    fanInfo.getFanU(),          "%.3f");
                gen.addNameValue("fanI",    fanInfo.getFanI(),          "%.3f");
            }
            gen.endObject();

            gen.beginObject("temperature");
            {
                gen.addNameValue("cold", g_envManager.getFanBoardTemp());
                gen.addNameValue("count",  g_envManager.getTempNum());
                gen.addNameValue("min",  g_envManager.getTempMin());
                gen.addNameValue("avr",  g_envManager.getTempAvg());
                gen.addNameValue("max",  g_envManager.getTempMax());
            }
            gen.endObject();

            gen.beginObject("slots");
            {
                for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
                {
                    for (int boardId = 0; boardId < MAX_BOARD_PER_SLAVE; boardId++)
                    {
                        BoardStat &board = g_masterStat.getSlave(slaveId).getBoardStat(boardId);
                        if (!board.isFound()) continue;

                        HistoryStatCalc calc(board.getBoardTotal(), HistoryStatMgr::PERIOD_DELTA);

                        snprintf(buffer, sizeof(buffer), "%d", board.getNum());
                        gen.beginObject(buffer);
                        {
                            gen.addNameValue("revision",        board.spec.revisionId);
                            gen.addNameValue("spiNum",          board.spec.spiNum);
                            gen.addNameValue("spiLen",          board.spec.spiLen);
                            gen.addNameValue("pwrNum",          board.spec.pwrNum);
                            gen.addNameValue("pwrLen",          board.spec.pwrLen);
                            gen.addNameValue("btcNum",          board.spec.btcNum);
                            gen.addNameValue("specVoltage",     board.spec.voltage);
                            gen.addNameValue("chips",           board.getBoardTotal().chips);
                            gen.addNameValue("temperature",     board.info.boardTemperature[0]);
                            gen.addNameValue("temperature1",    board.info.boardTemperature[1]);
                            gen.addNameValue("ocp",             board.info.overCurrentProtection);
                            gen.addNameValue("heaterErr",       board.info.heaterErr);
                            gen.addNameValue("heaterErrNum",    board.info.heaterErrNum);
                            gen.addNameValue("overheats",       board.info.ohNum);
                            gen.addNameValue("overheatsTime",   board.info.ohTime / 1000);
                            gen.addNameValue("lowCurrRst",      board.info.lowCurrRst);
                            gen.addNameValue("voltage",         board.info.voltage);

                            gen.beginArray("currents");
                            for (size_t i = 0; i < board.spec.pwrNum; i++)
                            {
                                gen.addValue(board.info.currents[i]);
                            }
                            gen.endArray();

                            gen.addNameValue("brokenPwc",       board.getBoardTotal().numBrokenPwc);

                            gen.addNameValue("solutions",       calc.getSol());
                            gen.addNameValue("errors",          calc.getErr());
                            gen.addNameValue("ghs",             calc.getGHs(), "%.1f");
                            gen.addNameValue("errorRate",       calc.getErrToSol(), "%.1f");
                            gen.addNameValue("chipRestarts",    calc.getChipRestarts());
                            gen.addNameValue("wattPerGHs",      util::safeDiv(board.getBoardTotal().power, calc.getGHs()));

                            gen.beginArray("tmpAlert");
                            {
                                for (int i = 0; i < MAX_TMP_PER_BOARD; i++)
                                {
                                    gen.beginObject();
                                    {
                                        gen.addNameValue("alertLo",     board.info.taInfo[i].alertLo);
                                        gen.addNameValue("alertHi",     board.info.taInfo[i].alertHi);
                                        gen.addNameValue("numWrite",    board.info.taInfo[i].numWrite);
                                    }
                                    gen.endObject();
                                }
                            }
                            gen.endArray();
                        }
                        gen.endObject();
                    }
                }
            }
            gen.endObject();

            gen.beginArray("slaves");
            {
                for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
                {
                    SlaveStat& slave = g_masterStat.getSlave(slaveId);

                    snprintf(buffer, sizeof(buffer), "0x%08x", slave.cmnInfo.swVersion);
                    int numRx = slave.msSpiRxOk + slave.msSpiRxError;

                    gen.beginObject();
                    {
                        gen.addNameValue("id",        slaveId);
                        gen.addNameValue("uid",       McuUIDToStr(slave.cmnInfo.uid).str);
                        gen.addNameValue("ver",       buffer);
                        gen.addNameValue("rx",        numRx);
                        gen.addNameValue("err",       numRx > 0 ? 100 * slave.msSpiRxError / numRx : 0);
                        gen.addNameValue("time",      slave.cmnInfo.totalTime);
                        gen.addNameValue("ping",      slave.getPingTime());
                    }
                    gen.endObject();
                }
            }
            gen.endArray();

            int pingMin = 0;
            int pingMax = 0;
            int pingAvg = 0;
            int pingN = 0;

            for (int slaveId = -1; EnvManager::nextSlaveId(slaveId); )
            {
                SlaveStat& slave = g_masterStat.getSlave(slaveId);
                int pingTime = slave.getPingTime();

                if (pingN == 0) {
                    pingMin = pingTime;
                    pingMax = pingTime;
                }
                else {
                    if (pingTime < pingMin) pingMin = pingTime;
                    if (pingTime > pingMax) pingMax = pingTime;
                }

                pingAvg += pingTime;
                pingN++;
            }

            pingAvg = (pingN > 0 ? pingAvg / pingN : 0);

            gen.addNameValue("slavePingMin", pingMin);
            gen.addNameValue("slavePingMax", pingMax);
            gen.addNameValue("slavePingAvg", pingAvg);
        }
        gen.endObject();

        fprintf(fd, "\n");
        fclose(fd);

        printf("[done]\n");
    }
    else {
        printf("ERROR: can not open file for write\n");
    }
}

