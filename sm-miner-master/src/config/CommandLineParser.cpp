/*
 * Contains CommandLineParser class definition.
 */

#include "CommandLineParser.h"

#include "base/MinMax.h"
#include "base/MiscUtil.h"
#include "base/TablePrinter.h"

#include "app/Application.h"

#include "config/Config.h"
#include "config/CommandLineException.h"

#include "board_revisions.h"

#include <stdarg.h>
#include <errno.h>
#include <stdio.h>


CommandLineParser::CommandLineParser(int argc, const char* argv[])
    : m_argc(argc)
    , m_argv(argv)
{
    assert(argc >= 0);
    assert(m_argv != nullptr);
}

void CommandLineParser::showUsage()
{
    const Config& defaults = Application::defaults();

    char slaveMaskStr[33] = {};
    uint32ToBinStr(defaults.slaveMask, slaveMaskStr, sizeof(slaveMaskStr));
    char boardMaskStr[33] = {};
    uint32ToBinStr(defaults.boardMask, boardMaskStr, sizeof(boardMaskStr));
    char boardMaskBStr[33] = {};
    uint32ToBinStr(defaults.boardMaskB, boardMaskBStr, sizeof(boardMaskBStr));

    printf("Usage (expected arguments):\n");
    
    printf("usage:\n");
    printf("  -help                     Display this screen (command line usage)\n");
    
    printf("pool:\n");
    printf("  -host <hostname>          Set pool hostname (default: [%s])\n", defaults.poolConfig.host.cdata());
    printf("  -port <port>              Set pool port     (default: [%s])\n", defaults.poolConfig.port.cdata());
    printf("  -user <username>          Set pool username (default: [%s])\n", defaults.poolConfig.userName.cdata());
    printf("  -pswd <password>          Set pool password (default: [%s])\n", defaults.poolConfig.password.cdata());
	#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180313
    // bak1 pool
    printf("  -bak1_host <hostname>          Set bak1 pool hostname (default: [%s])\n", defaults.poolConfig.bak1Host.cdata());
    printf("  -bak1_port <port>              Set bak1 pool port     (default: [%s])\n", defaults.poolConfig.bak1Port.cdata());
    printf("  -bak1_user <username>          Set bak1 pool username (default: [%s])\n", defaults.poolConfig.bak1UserName.cdata());
    printf("  -bak1_pswd <password>          Set bak1 pool password (default: [%s])\n", defaults.poolConfig.bak1Password.cdata());
    // bak2 pool
    printf("  -bak2_host <hostname>          Set bak2 pool hostname (default: [%s])\n", defaults.poolConfig.bak2Host.cdata());
    printf("  -bak2_port <port>              Set bak2 pool port     (default: [%s])\n", defaults.poolConfig.bak2Port.cdata());
    printf("  -bak2_user <username>          Set bak2 pool username (default: [%s])\n", defaults.poolConfig.bak2UserName.cdata());
    printf("  -bak2_pswd <password>          Set bak2 pool password (default: [%s])\n", defaults.poolConfig.bak2Password.cdata());
    // Retry n times, then re-select pool
    printf("  -retry_n <n>          Retry n times, then re-select pool (default: [%u])\n", defaults.poolConfig.retryNTimes);
    // fcj add end
    #endif
    printf("  -exit_on_errors           Exit if pool rejects our shares\n");
    printf("  -connect_attempts <n>     Exit if cannot connect after <n> attempts (default: %d)\n", defaults.poolConfig.maxConnectionAttempts);
    printf("  -auth_t <n>               Authorization timeout in seconds (default: %u)\n", defaults.poolConfig.authorizationTimeoutSec);
    printf("  -subscribe_t <n>          Subscription timeout in seconds (default: %u)\n", defaults.poolConfig.subscriptionTimeoutSec);
    printf("  -start_diff <n>           Initial pool difficulty (default: %u)\n", defaults.poolConfig.initialDifficulty);

    printf("psu:\n");
    printf("  -no_psu <x>               No PSU (external power): do not wait for power ack\n");
    printf("  -psu_fan_user             PSU FAN: user speed control\n");
    printf("  -psu_fan                  PSU FAN: speed in percents (default: %u%%)\n", defaults.psuConfig.fanSpeedValue);
    printf("  -psu_v <v>                PSU: full power voltage (default: %.1fV)\n", defaults.psuConfig.getVoltage());
    printf("  -psu_min_v <v>            PSU: minimal voltage (default: %.1fV)\n", defaults.psuConfig.getMinimalVoltage());
    printf("  -psu_t <sec>              PSU: on/off delay per unit (default: %u sec)\n", defaults.psuConfig.delaySec);
    printf("  -psu_start_v <v>          PSU: initial voltage (default: %.1fV)\n", defaults.psuConfig.getInitialVoltage());
    printf("  -psu_start_t <sec>        PSU: initial voltage hold time (default: %u sec)\n", defaults.psuConfig.lowPowerTime);
    printf("  -psu_dv <vp> <vn>         PSU: inc/dec voltage per second (default: %.1f/%.1f V)\n", defaults.psuConfig.getVoltageStepUp(), defaults.psuConfig.getVoltageStepDown());
    printf("  -psu_max_i <i> <di>       PSU: current limit thresholds (default: %.1f +/- %.1f A)\n", defaults.psuConfig.getCurrentLimit(), defaults.psuConfig.getCurrentLimitDelta());

    printf("psu log:\n");
    printf("  -pl_on <n>                PSU log: turn on(1)/off(0) logging (default: %u)\n", defaults.psuLogConfig.logOn);
    printf("  -pl_all <n>               PSU log: log all messages (default: %u)\n", defaults.psuLogConfig.logAll);
    printf("  -pl_depth <n>             PSU log: msgs to log before and after event (default: %u)\n", defaults.psuLogConfig.logDepth);
    printf("  -pl_dev <k>               PSU log: deviation from prev status (default: %.2f)\n", defaults.psuLogConfig.deviation);
    printf("  -pl_vi_min <V>            PSU log: V in min (default: %.1fV)\n", defaults.psuLogConfig.vInMin);
    printf("  -pl_vi_max <V>            PSU log: V in max (default: %.1fV)\n", defaults.psuLogConfig.vInMax);
    printf("  -pl_vo_min <V>            PSU log: V out min (default: %.1fV)\n", defaults.psuLogConfig.vOutMin);
    printf("  -pl_vo_max <V>            PSU log: V out max (default: %.1fV)\n", defaults.psuLogConfig.vOutMax);
    printf("  -pl_io_min <A>            PSU log: I out min (default: %.1fA)\n", defaults.psuLogConfig.iOutMin);
    printf("  -pl_io_max <A>            PSU log: I out max (default: %.1fA)\n", defaults.psuLogConfig.iOutMax);

    printf("grid/env:\n");
    printf("  -mb_hw_ver <dm>           Use mb hw ver: 0xff-auto, 0xf-6 boards, 0x1-24 boards (default: %d)\n", defaults.mbHwVer);
    printf("  -slave_mask <bin>         Set slave mask (default: %s)\n", slaveMaskStr);
    printf("  -board_mask <bin>         Set board mask (type A + B) (default: %s)\n", boardMaskStr);
    printf("  -board_mask_b <bin>       Set board mask (type B) (default: %s)\n", boardMaskBStr);
    printf("  -slave_spi_drv <dm>       Master SPI: drv mode 0-hw, 1-sw, 2-auto (default: %d)\n", defaults.slaveSpiDrv);
    printf("  -slave_spi_speed <n>      Master SPI: speed in Hz (default: %d)\n", defaults.slaveSpiSpeed);
    printf("  -ms_spi_debug             Master-Slave SPI debug mode\n");
    printf("  -psu_v <x>                Set PSU voltage (default: %.1f)\n", defaults.psuConfig.getVoltage());
    printf("  -fan_v <x>                Set FAN voltage (default: %.1f)\n", defaults.slaveConfig.fanConfig.getFanVoltage());

    printf("stats export/events:\n");
    printf("  -alive_event_dt <n>       Report miner-alive event each <n> seconds (default: %d)\n", defaults.aliveEventIntervalSec);

    printf("pwc config:\n");
    printf("  -spi_t_rst <t0>           SPI timing rst in MCU clycles (default: %d)\n",
           defaults.slaveConfig.spiTimeRst);
    printf("  -spi_t_tx <t0/t1/t2>      SPI timing tx time in MCU clycles (default: %d/%d/%d)\n",
           defaults.slaveConfig.spiTimeTx0, defaults.slaveConfig.spiTimeTx1, defaults.slaveConfig.spiTimeTx2);
    printf("  -spi_t_rx <t0/t1>         SPI timing rx time in MCU clycles (default: %d/%d)\n",
           defaults.slaveConfig.spiTimeRx0, defaults.slaveConfig.spiTimeRx1);
    printf("  -spi_reset <cycles>       SPI chip reset cycles (default: %d)\n", defaults.slaveConfig.spiResetCycles);
    printf("  -pwc_clock_cfg <n>        PWC cpu clock cfg (default: %u)\n", defaults.slaveConfig.pwcClockCfg);
    printf("                              cfg/MHz: 0/82, 32/136, 40/165, 42/172\n");

    printf("chip:\n");
    printf("  -osc <n>                  OSC: static oscillator register (default: %d)\n", defaults.pwcConfig.osc);
    printf("  -jt <n>                   OSC: base job time in ms (default: %d)\n", defaults.pwcConfig.jt);
    printf("  -ao_mode <n>              Auto OSC: mode: 0 - static, 1 - rough osc (default: %d)\n", defaults.pwcConfig.oscMode);
    printf("  -ro_osc_min <n>           Rough OSC: min osc (default: %d)\n", defaults.pwcConfig.roOscMin);
    printf("  -ro_osc_max <n>           Rough OSC: max osc (default: %d)\n", defaults.pwcConfig.roOscMax);
    printf("  -ro_eff_z0 <n>            Rough OSC: zone 0 efficiency level (to dec osc) (default: %d)\n", defaults.pwcConfig.roEffZone0);
    printf("  -ro_eff_z1 <n>            Rough OSC: zone 1 efficiency level (to inc osc) (default: %d)\n", defaults.pwcConfig.roEffZone1);
    printf("  -ro_deltas_dec <n>        Rough OSC: deltas to stay at zone to dec osc (default: %d)\n", defaults.pwcConfig.roDeltasToDec);
    printf("  -ro_deltas_inc <n>        Rough OSC: deltas to stay at zone to inc osc (default: %d)\n", defaults.pwcConfig.roDeltasToInc);

    printf("slave mcu:\n");
    printf("  -mcu_log_chip_stat        MCU serial: log chip stats\n");
    printf("  -mcu_log_stratum          MCU serial: log stratum\n");
    printf("  -mcu_log_spi <spi> <seq>  MCU serial: log spi data from chip\n");
    printf("  -slave_log_adc            Log all ADC periodically\n");
    printf("  -slave_test_pwr_sw        Turn on/off power switch periodically\n");

    printf("pwc:\n");
    printf("  -pwc_btc_ser_cfg <n>      BTC serial config (default: 0x%08x)\n", defaults.pwcConfig.btcSerCfg);

    printf("board:\n");
    printf("  -brd_ocp <n>              Over Current Protection: max current per power line (default: 0x%02x)\n", defaults.slaveConfig.overCurrentProtection);
    printf("  -brd_pwr_off <n>          Turn OFF board power (0 based sequence)\n");
    printf("  -brd_pwr_off_all          Turn OFF all boards power\n");
    printf("board user spec:\n");
    printf("  -user_slot_mask <bin>     User slot mask (example: 101)\n");
    printf("  -user_brd_rev <n>         User board: revision id\n");
    printf("  -user_brd_if_found        User board: apply config if board detected\n");
    printf("  -user_brd_spi_num <n>     User board: num of spi\n");
    printf("  -user_brd_spi_len <n>     User board: len of spi\n");
    printf("  -user_brd_spi_mask <bin>  User board: spi lines mask (example: 101)\n");
    printf("  -user_brd_pwr_num <n>     User board: num of power lines\n");
    printf("  -user_brd_pwr_len <n>     User board: len of power lines\n");
    printf("  -user_brd_btc_num <n>     User board: num of btc per power chip\n");
    printf("  -user_brd_btc_mask <bin>  User board: btc lines mask (example: 01111110)\n");
    printf("board pwr switch monitoring:\n");
    printf("  -ps_oh_curr <amp>         Low current threshold (0-no cooling) (default: %d ma)\n", defaults.slaveConfig.psLowCurrent);
    printf("  -ps_oh_t <sec>            Low current period (default: %d sec)\n", defaults.slaveConfig.psLowCurrentPeriod);
    printf("  -ps_temp_lo <C>           Lo temp threshold to allow power ON (default: %uC)\n", defaults.slaveConfig.maxTempLo);
    printf("  -ps_temp_hi <C>           Hi temp threshold to power OFF (default: %uC)\n", defaults.slaveConfig.maxTempHi);

    printf("other:\n");
    printf("  -polling_delay <ms>       Polling delay in milliseconds (default: %d)\n", defaults.pollingDelayMs);
    
    printf("statistics:\n");
    printf("  -log_delay <sec>          Logging delay in seconds (default: %d)\n", defaults.logDelaySec);
    printf("  -log_chip_stat            Log chip statistics\n");
    
    printf("logging:\n");
    printf("  -trace                    Enable trace level logging\n");
    printf("  -trace_stratum            Dump stratum protocol communication\n");
    printf("  -log_slave <level>        Dump slave mcu protocol communication\n");
    
    printf("test modes:\n");
    printf("  -test                     Server test mode\n");
    printf("  -test_cb                  Chip Board test mode\n");
    printf("  -test_mb                  Mother Board flashing / test mode\n");
    printf("  -test_fb                  Fan Board flashing / test mode\n");
    printf("slave test config:\n");
    printf("  -test_btc_t <ms>          Test: btc test time (default: %d ms)\n", defaults.slaveConfig.hbtConfig.testBtcTime);
    printf("  -test_fan_dt <ms>         Test: delay between pwr setup and measurements (default: %d ms)\n", defaults.slaveConfig.hbtConfig.testFanDelay);
    printf("test options:\n");
    printf("  -test_num <n>             Test: number of test iterations (default: %d)\n", defaults.testConfig.testNum);
    printf("  -test_t <sec>             Test: time in seconds (default: %d sec)\n", defaults.testConfig.testTimeSec);
    printf("  -test_cooling_t <sec>     Test: timeout to cool boards (default: %d sec)\n", defaults.testConfig.testCoolingTimeSec);
    printf("  -test_fan_wait_i <a>      Test: wait fan current before test (default: %.1f A)\n", defaults.testConfig.testFanWaitI);
    printf("  -test_mb_hw_ver <n>       Test MB: hw ver (default: 0x%x)\n", defaults.testConfig.testMbHwVer);
    printf("  -test_slave_hw_ver <n>    Test NB: slave hw ver (default: 0x%x)\n", defaults.testConfig.testSlaveHwVer);
    printf("  -test_brd_rev <n>         Test BRD: expected revision (default: %d)\n", defaults.testConfig.testExpRev);
    printf("  -test_temp_min <n>        Test BRD: min temperature (default: %d)\n", defaults.testConfig.testTempMin);
    printf("  -test_temp_max <n>        Test BRD: max temperature (default: %d)\n", defaults.testConfig.testTempMax);
    printf("  -test_no_hs               Test BRD: missing heatsink\n");
    printf("  -test_no_hs_n_max <n>     Test BRD: max number of heatsink err pin transitions (default: %d)\n", defaults.testConfig.testNoHsNumMax);
    printf("  -test_hs_n_min <n>        Test BRD: min number of heatsink err pin transitions (default: %d)\n", defaults.testConfig.testHsNumMin);
    printf("  -test_volt_min <n>        Test BRD: min voltage (default: %d)\n", defaults.testConfig.testVoltageMin);
    printf("  -test_volt_max <n>        Test BRD: max voltage (default: %d)\n", defaults.testConfig.testVoltageMax);
    printf("  -test_curr_min <n>        Test BRD: min current (default: %d)\n", defaults.testConfig.testCurrentMin);
    printf("  -test_curr_max <n>        Test BRD: max current (default: %d)\n", defaults.testConfig.testCurrentMax);
    printf("  -test_volt2_min <n>       Test BRD: min voltage (default: %d)\n", defaults.testConfig.testVoltage2Min);
    printf("  -test_volt2_max <n>       Test BRD: max voltage (default: %d)\n", defaults.testConfig.testVoltage2Max);
    printf("  -test_curr2_min <n>       Test BRD: min current (default: %d)\n", defaults.testConfig.testCurrent2Min);
    printf("  -test_curr2_max <n>       Test BRD: max current (default: %d)\n", defaults.testConfig.testCurrent2Max);
    printf("  -test_ocp_min <n>         Test BRD: min ocp off (default: 0x%02x)\n", defaults.testConfig.testOcpOffMin);
    printf("  -test_ocp_max <n>         Test BRD: max ocp off (default: 0x%02x)\n", defaults.testConfig.testOcpOffMax);
    printf("  -tm_btc_no_sol <n>        Test BTC: max no solutions chips (default: %d)\n", defaults.testConfig.testMaxBtcNoSol);
    printf("  -tm_btc_no_js <n>         Test BTC: max no job switch chips (default: %d)\n", defaults.testConfig.testMaxBtcNoJs);
    printf("  -tm_btc_no_resp <n>       Test BTC: max no response chips (default: %d)\n", defaults.testConfig.testMaxBtcNoResp);
    printf("  -test_warn <0|1>          Test: print warnings (default: %d)\n", defaults.testConfig.testPrintWarnings);
    printf("fan test config:\n");
    printf("  -test_fan_on_v_dev <k>    Fan test: fan on, max voltage deviation get/cfg (default: %.2f)\n", defaults.testConfig.testFanOnVDev);
    printf("  -test_fan_on_i_min <a>    Fan test: fan on, min current (default: %.1f A)\n", defaults.testConfig.testFanOnIMin);
    printf("  -test_fan_on_i_max <a>    Fan test: fan on, max current (default: %.1f A)\n", defaults.testConfig.testFanOnIMax);

    printf("special:\n");
    printf("  -no_mcu_reset             Disable MCU reset on start-up\n");
    
    printf("log levels:\n");
    printf("  0 - NONE, 1 - INFO, 2 - DEBUG, 3 - TRACE\n");

    printf("\n");


    TablePrinter tp;
    tp.writeCell("Rev ADC");
    tp.writeCell("Rev");
    tp.writeCell("SPI (mask)");
    tp.writeCell("PWR");
    tp.writeCell("BTC (mask)");
    tp.writeCell("Volt");
    tp.writeCell("Flags");
    tp.writeCell("adcKu");
    tp.writeCell("adcKi");
    tp.writeCell("Human Name");
    tp.newLine();

    for (int i = 0; true; i++)
    {
        RevisionData *rd = BoardRevisions::getRevisionByIndex(i);
        if (!rd) break;

        BoardSpec &s = rd->spec;

        tp.writeCell("%d - %d", rd->revisionAdcMin, rd->revisionAdcMax);
        tp.writeCell("%d", s.revisionId);
        tp.writeCell("%d x %d (0x%x)", s.spiNum, s.spiLen, s.spiMask);
        tp.writeCell("%d x %d", s.pwrNum, s.pwrLen);
        tp.writeCell("%d (0x%x)", s.btcNum, s.btcMask);
        tp.writeCell("%d", s.voltage);
        tp.writeCell("0x%02x", s.flags);
        tp.writeCell("%.2f", s.adcKu);
        tp.writeCell("%.2f", s.adcKi);
        tp.writeCell("%s", s.humanName);
        tp.newLine();
    }

    printf("Board revisions table:\n");
    tp.printTable();


    printf("\n");
}

void CommandLineParser::parse(int argc, const char* argv[], Config& config)
{
    CommandLineParser parser(argc, argv);

    int argn = 1;
    while (const char* const option = parser.getArgument(argn++))
    {
        /////////////////////
        // Runtime options.
        
        if (optionEqual(option, "help"))
        {
            config.showUsage = true;
        }

        /////////////////////
        // Pool options.
        
        else if (optionEqual(option, "host"))
        {
            config.poolConfig.host = parser.getParamAsStr(argn++, option, "hostname");
        }
        else if (optionEqual(option, "port"))
        {
            config.poolConfig.port = parser.getParamAsStr(argn++, option, "port");
        }
        else if (optionEqual(option, "user"))
        {
            config.poolConfig.userName = parser.getParamAsStr(argn++, option, "username");
        }
        else if (optionEqual(option, "pswd"))
        {
            config.poolConfig.password = parser.getParamAsStr(argn++, option, "password");
        }
#ifdef MULTI_POOL_SUPPORT
	// fcj add begin 20180313
	// bak1 pool
        else if (optionEqual(option, "bak1_host"))
        {
            config.poolConfig.bak1Host = parser.getParamAsStr(argn++, option, "hostname");
        }
        else if (optionEqual(option, "bak1_port"))
        {
            config.poolConfig.bak1Port = parser.getParamAsStr(argn++, option, "port");
        }
        else if (optionEqual(option, "bak1_user"))
        {
            config.poolConfig.bak1UserName = parser.getParamAsStr(argn++, option, "username");
        }
        else if (optionEqual(option, "bak1_pswd"))
        {
            config.poolConfig.bak1Password = parser.getParamAsStr(argn++, option, "password");
        }

	// bak2 pool
        else if (optionEqual(option, "bak2_host"))
        {
            config.poolConfig.bak2Host = parser.getParamAsStr(argn++, option, "hostname");
        }
        else if (optionEqual(option, "bak2_port"))
        {
            config.poolConfig.bak2Port = parser.getParamAsStr(argn++, option, "port");
        }
        else if (optionEqual(option, "bak2_user"))
        {
            config.poolConfig.bak2UserName = parser.getParamAsStr(argn++, option, "username");
        }
        else if (optionEqual(option, "bak2_pswd"))
        {
            config.poolConfig.bak2Password = parser.getParamAsStr(argn++, option, "password");
        }

	// Retry n times, then re-select pool
        else if (optionEqual(option, "retry_n"))
        {
            config.poolConfig.retryNTimes = parser.getParamAsUInt(argn++, option, "n");
        }
	// fcj add end
#endif

        else if (optionEqual(option, "exit_on_errors"))
        {
            config.poolConfig.exitOnError = true;
        }
        else if (optionEqual(option, "connect_attempts"))
        {
            config.poolConfig.maxConnectionAttempts = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "auth_t"))
        {
            config.poolConfig.authorizationTimeoutSec = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "subscribe_t"))
        {
            config.poolConfig.subscriptionTimeoutSec = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "start_diff"))
        {
            config.poolConfig.initialDifficulty = parser.getParamAsUInt32(argn++, option, "n");
        }

        /////////////////////
        // General application options.
        
        else if (optionEqual(option, "polling_delay"))
        {
            config.pollingDelayMs = parser.getParamAsUInt(argn++, option, "ms");
        }
        else if (optionEqual(option, "alive_event_dt"))
        {
            config.aliveEventIntervalSec = parser.getParamAsUInt(argn++, option, "sec");
        }

        /////////////////////
        // PSU options.

        else if (optionEqual(option, "no_psu"))
        {
            config.psuConfig.noPsu = true;
        }
        else if (optionEqual(option, "psu_fan_user"))
        {
            config.psuConfig.fanSpeedUser = true;
        }
        else if (optionEqual(option, "psu_fan"))
        {
            config.psuConfig.fanSpeedValue = parser.getParamAsUInt8(argn++, option, "%%");;
        }
        else if (optionEqual(option, "psu_v"))
        {
            config.psuConfig.setVoltage( parser.getParamAsDouble(argn++, option, "V") );
        }
        else if (optionEqual(option, "psu_min_v"))
        {
            config.psuConfig.setMinimalVoltage( parser.getParamAsDouble(argn++, option, "V") );
        }
        else if (optionEqual(option, "psu_t"))
        {
            config.psuConfig.delaySec = parser.getParamAsUInt16(argn++, option, "sec");
        }

        else if (optionEqual(option, "psu_start_v"))
        {
            config.psuConfig.setInitialVoltage( parser.getParamAsDouble(argn++, option, "V") );
        }
        else if (optionEqual(option, "psu_start_t"))
        {
            config.psuConfig.lowPowerTime = parser.getParamAsUInt16(argn++, option, "sec");
        }

        else if (optionEqual(option, "psu_dv"))
        {
            config.psuConfig.setVoltageStepUp( parser.getParamAsDouble(argn++, option, "V") );
            config.psuConfig.setVoltageStepDown( parser.getParamAsDouble(argn++, option, "V") );
        }
        else if (optionEqual(option, "psu_max_i"))
        {
            config.psuConfig.setCurrentLimit( parser.getParamAsDouble(argn++, option, "A") );
            config.psuConfig.setCurrentLimitDelta( parser.getParamAsDouble(argn++, option, "A") );
        }

        /////////////////////
        // PSU logging

        else if (optionEqual(option, "pl_on"))
        {
            config.psuLogConfig.logOn = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "pl_all"))
        {
            config.psuLogConfig.logAll = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "pl_depth"))
        {
            config.psuLogConfig.logDepth = parser.getParamAsUInt16(argn++, option, "n");
        }
        else if (optionEqual(option, "pl_dev"))
        {
            config.psuLogConfig.deviation = parser.getParamAsDouble(argn++, option, "k");
        }

        else if (optionEqual(option, "pl_vi_min"))
        {
            config.psuLogConfig.vInMin = parser.getParamAsDouble(argn++, option, "V");
        }
        else if (optionEqual(option, "pl_vi_max"))
        {
            config.psuLogConfig.vInMax = parser.getParamAsDouble(argn++, option, "V");
        }

        else if (optionEqual(option, "pl_vo_min"))
        {
            config.psuLogConfig.vOutMin = parser.getParamAsDouble(argn++, option, "V");
        }
        else if (optionEqual(option, "pl_vo_max"))
        {
            config.psuLogConfig.vOutMax = parser.getParamAsDouble(argn++, option, "V");
        }

        else if (optionEqual(option, "pl_io_min"))
        {
            config.psuLogConfig.iOutMin = parser.getParamAsDouble(argn++, option, "A");
        }
        else if (optionEqual(option, "pl_io_max"))
        {
            config.psuLogConfig.iOutMax = parser.getParamAsDouble(argn++, option, "A");
        }

        /////////////////////
        // Grig options.
        
        else if (optionEqual(option, "mb_hw_ver"))
        {
            config.mbHwVer = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "slave_mask"))
        {
            config.slaveMask = binStrToUInt32( parser.getParamAsStr(argn++, option, "bin") );
            ::printf("Slave mask: 0x%04x\n", config.slaveMask);
        }
        else if (optionEqual(option, "board_mask"))
        {
            config.boardMask = binStrToUInt32( parser.getParamAsStr(argn++, option, "bin") );
            ::printf("Board mask: 0x%04x\n", config.boardMask);
        }
        else if (optionEqual(option, "board_mask_b"))
        {
            config.boardMaskB = binStrToUInt32( parser.getParamAsStr(argn++, option, "bin") );
            ::printf("Board mask B: 0x%04x\n", config.boardMaskB);
        }
        else if (optionEqual(option, "slave_spi_drv"))
        {
            config.slaveSpiDrv = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "slave_spi_speed"))
        {
            config.slaveSpiSpeed = parser.getParamAsUInt32(argn++, option, "hz");
        }
        else if (optionEqual(option, "ms_spi_debug"))
        {
            config.msSpiDebug = true;
        }
        else if (optionEqual(option, "fan_v"))
        {
            config.slaveConfig.fanConfig.setFanVoltage( parser.getParamAsDouble(argn++, option, "x") );
        }

        /////////////////////
        // SPI options.

        else if (optionEqual(option, "spi_t_rst"))
        {
            config.slaveConfig.spiTimeRst = parser.getParamAsUInt16(argn++, option, "cycles");
        }
        else if (optionEqual(option, "spi_t_tx"))
        {
            config.slaveConfig.spiTimeTx0 = parser.getParamAsUInt16(argn++, option, "cycles");
            config.slaveConfig.spiTimeTx1 = parser.getParamAsUInt16(argn++, option, "cycles");
            config.slaveConfig.spiTimeTx2 = parser.getParamAsUInt16(argn++, option, "cycles");
        }
        else if (optionEqual(option, "spi_t_rx"))
        {
            config.slaveConfig.spiTimeRx0 = parser.getParamAsUInt16(argn++, option, "cycles");
            config.slaveConfig.spiTimeRx1 = parser.getParamAsUInt16(argn++, option, "cycles");
        }
        else if (optionEqual(option, "spi_reset"))
        {
            config.slaveConfig.spiResetCycles = parser.getParamAsUInt16(argn++, option, "cycles");
        }
        else if (optionEqual(option, "pwc_clock_cfg"))
        {
            config.slaveConfig.pwcClockCfg = parser.getParamAsUInt8(argn++, option, "n");
        }

        /////////////////////
        // Chip options.
        
        else if (optionEqual(option, "osc"))
        {
            config.pwcConfig.osc = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "jt"))
        {
            config.pwcConfig.jt = parser.getParamAsUInt16(argn++, option, "n");
        }

        else if (optionEqual(option, "ao_mode"))
        {
            config.pwcConfig.oscMode = parser.getParamAsUInt8(argn++, option, "n");
        }

        else if (optionEqual(option, "ro_osc_min"))
        {
            config.pwcConfig.roOscMin = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "ro_osc_max"))
        {
            config.pwcConfig.roOscMax = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "ro_eff_z0"))
        {
            config.pwcConfig.roEffZone0 = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "ro_eff_z1"))
        {
            config.pwcConfig.roEffZone1 = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "ro_deltas_dec"))
        {
            config.pwcConfig.roDeltasToDec = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "ro_deltas_inc"))
        {
            config.pwcConfig.roDeltasToInc = parser.getParamAsUInt8(argn++, option, "n");
        }

        /////////////////////
        // MCU options.
        
        else if (optionEqual(option, "mcu_log_chip_stat"))
        {
            config.slaveConfig.mcuLogChipStat = true;
        }
        else if (optionEqual(option, "mcu_log_stratum"))
        {
            config.slaveConfig.mcuLogStratum = true;
        }
        else if (optionEqual(option, "mcu_log_spi"))
        {
            config.slaveConfig.mcuLogSpi = true;
            
            config.slaveConfig.mcuLogSpi_spi = parser.getParamAsUInt8(argn++, option, "spi");
            config.slaveConfig.mcuLogSpi_seq = parser.getParamAsUInt8(argn++, option, "seq");
        }

        else if (optionEqual(option, "slave_log_adc"))
        {
            config.slaveConfig.debugOpt |= SLAVE_DEBUG_LOG_ADC;
        }
        else if (optionEqual(option, "slave_test_pwr_sw"))
        {
            config.slaveConfig.debugOpt |= SLAVE_DEBUG_TEST_POWER_SW;
        }

        /////////////////////
        // PWC options.

        else if (optionEqual(option, "pwc_btc_ser_cfg"))
        {
            config.pwcConfig.btcSerCfg = parser.getParamAsUInt32(argn++, option, "n");
        }

        /////////////////////
        // Board options.
        
        else if (optionEqual(option, "brd_ocp"))
        {
            config.slaveConfig.overCurrentProtection = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "brd_pwr_off"))
        {
            int slotId = parser.getParamAsUInt8(argn++, option, "n");

            if (slotId >= MAX_BOARD_PER_MASTER)
            {
                throw CommandLineException("Parameter value %s = %lu is out of range",
                    option, slotId);
            }

//            config.boardConfig[slotId].powerOn = false;
        }
        else if (optionEqual(option, "brd_pwr_off_all"))
        {
            for (int slotId = 0; slotId < MAX_BOARD_PER_MASTER; slotId++)
            {
                config.boardConfig[slotId].powerOn = false;
            }
        }

        else if (optionEqual(option, "user_slot_mask"))
        {
            config.slaveConfig.userSlotMask = binStrToUInt32( parser.getParamAsStr(argn++, option, "bin") );
        }
        else if (optionEqual(option, "user_brd_rev"))
        {
            config.slaveConfig.userBrdRev = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "user_brd_if_found"))
        {
            config.slaveConfig.userBrdIfFound = true;
        }
        else if (optionEqual(option, "user_brd_spi_num"))
        {
            config.slaveConfig.userBrdSpiNum = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "user_brd_spi_len"))
        {
            config.slaveConfig.userBrdSpiLen = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "user_brd_spi_mask"))
        {
            config.slaveConfig.userBrdSpiMask = binStrToUInt32( parser.getParamAsStr(argn++, option, "bin") );
        }
        else if (optionEqual(option, "user_brd_pwr_num"))
        {
            config.slaveConfig.userBrdPwrNum = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "user_brd_pwr_len"))
        {
            config.slaveConfig.userBrdPwrLen = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "user_brd_btc_num"))
        {
            config.slaveConfig.userBrdBtcNum = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "user_brd_btc_mask"))
        {
            config.slaveConfig.userBrdBtcMask = binStrToUInt32( parser.getParamAsStr(argn++, option, "bin") );
        }

        else if (optionEqual(option, "ps_oh_curr"))
        {
            config.slaveConfig.psLowCurrent = parser.getParamAsUInt16(argn++, option, "amp");
        }
        else if (optionEqual(option, "ps_oh_t"))
        {
            config.slaveConfig.psLowCurrentPeriod = parser.getParamAsUInt8(argn++, option, "sec");
        }
        else if (optionEqual(option, "ps_temp_lo"))
        {
            config.slaveConfig.maxTempLo = parser.getParamAsUInt8(argn++, option, "C");
        }
        else if (optionEqual(option, "ps_temp_hi"))
        {
            config.slaveConfig.maxTempHi = parser.getParamAsUInt8(argn++, option, "C");
        }

        /////////////////////
        // Statistics options.
        
        else if (optionEqual(option, "log_delay"))
        {
            config.logDelaySec = parser.getParamAsUInt(argn++, option, "sec");
        }
        else if (optionEqual(option, "log_chip_stat"))
        {
            config.logChipStat = true;
        }
        else if (optionEqual(option, "log_empty_boards"))
        {
            config.logEmptyBoards = true;
        }

        /////////////////////
        // Logging options.
        
        else if (optionEqual(option, "trace"))
        {
            config.traceAll = true;
        }
        else if (optionEqual(option, "trace_stratum")
            || optionEqual(option, "log_stratum"))  // deprecated, left for compatibility
        {
            config.traceStratum = true;
        }
        else if (optionEqual(option, "log_slave"))
        {
            config.logSlaveLevel = parser.getParamAsUInt(argn++, option, "level");
        }

        /////////////////////
        // Server Test mode.
        
        else if (optionEqual(option, "test"))
        {
            config.testConfig.testMode = TEST_MODE_SERVER;
        }
        else if (optionEqual(option, "test_cb"))
        {
            config.testConfig.testMode = TEST_MODE_HASH_BOARD;
        }
        else if (optionEqual(option, "test_mb"))
        {
            config.testConfig.testMode = TEST_MODE_MOTHER_BOARD;
        }
        else if (optionEqual(option, "test_fb"))
        {
            config.testConfig.testMode = TEST_MODE_FAN_BOARD;
        }

        /////////////////////
        // Server Test options.
        
        else if (optionEqual(option, "test_btc_t"))
        {
            config.slaveConfig.hbtConfig.testBtcTime = parser.getParamAsUInt16(argn++, option, "ms");
        }
        else if (optionEqual(option, "test_fan_dt"))
        {
            config.slaveConfig.hbtConfig.testFanDelay = parser.getParamAsUInt16(argn++, option, "ms");
        }

        else if (optionEqual(option, "test_num"))
        {
            config.testConfig.testNum = parser.getParamAsUInt8(argn++, option, "n");
        }
        else if (optionEqual(option, "test_t"))
        {
            config.testConfig.testTimeSec = parser.getParamAsUInt(argn++, option, "sec");
        }
        else if (optionEqual(option, "test_cooling_t"))
        {
            config.testConfig.testCoolingTimeSec = parser.getParamAsUInt(argn++, option, "sec");
        }
        else if (optionEqual(option, "test_fan_wait_i")) {
            config.testConfig.testFanWaitI = parser.getParamAsDouble(argn++, option, "a");
        }
        else if (optionEqual(option, "test_mb_hw_ver"))
        {
            config.testConfig.testMbHwVer = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_slave_hw_ver"))
        {
            config.testConfig.testSlaveHwVer = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_brd_rev"))
        {
            config.testConfig.testExpRev = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_temp_min"))
        {
            config.testConfig.testTempMin = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_temp_max"))
        {
            config.testConfig.testTempMax = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_no_hs"))
        {
            config.testConfig.testNoHeatSink = true;
        }
        else if (optionEqual(option, "test_no_hs_n_max"))
        {
            config.testConfig.testNoHsNumMax = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_hs_n_min"))
        {
            config.testConfig.testHsNumMin = parser.getParamAsUInt(argn++, option, "n");
        }

        else if (optionEqual(option, "test_volt_min"))
        {
            config.testConfig.testVoltageMin = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_volt_max"))
        {
            config.testConfig.testVoltageMax = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_curr_min"))
        {
            config.testConfig.testCurrentMin = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_curr_max"))
        {
            config.testConfig.testCurrentMax = parser.getParamAsUInt(argn++, option, "n");
        }

        else if (optionEqual(option, "test_volt2_min"))
        {
            config.testConfig.testVoltage2Min = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_volt2_max"))
        {
            config.testConfig.testVoltage2Max = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_curr2_min"))
        {
            config.testConfig.testCurrent2Min = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_curr2_max"))
        {
            config.testConfig.testCurrent2Max = parser.getParamAsUInt(argn++, option, "n");
        }

        else if (optionEqual(option, "test_ocp_min"))
        {
            config.testConfig.testOcpOffMin = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_ocp_max"))
        {
            config.testConfig.testOcpOffMax = parser.getParamAsUInt(argn++, option, "n");
        }

        else if (optionEqual(option, "tm_btc_no_sol"))
        {
            config.testConfig.testMaxBtcNoSol = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "tm_btc_no_js"))
        {
            config.testConfig.testMaxBtcNoJs = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "tm_btc_no_resp"))
        {
            config.testConfig.testMaxBtcNoResp = parser.getParamAsUInt(argn++, option, "n");
        }
        else if (optionEqual(option, "test_warn"))
        {
            config.testConfig.testPrintWarnings = parser.getParamAsUInt(argn++, option, "n");
        }

        else if (optionEqual(option, "test_fan_on_v_dev")) {
            config.testConfig.testFanOnVDev = parser.getParamAsDouble(argn++, option, "k");
        }
        else if (optionEqual(option, "test_fan_on_i_min")) {
            config.testConfig.testFanOnIMin = parser.getParamAsDouble(argn++, option, "a");
        }
        else if (optionEqual(option, "test_fan_on_i_max")) {
            config.testConfig.testFanOnIMax = parser.getParamAsDouble(argn++, option, "a");
        }

        /////////////////////
        // Special options.
        
        else if (optionEqual(option, "no_mcu_reset"))
        {
            config.noMcuReset = true;
        }

        /////////////////////
        // Unknown command line option.
        
        else
        {
            throw CommandLineException("Unknown command line option: %s", option);
        }
    }

    config.slaveConfig.testMode = static_cast<uint8_t>(config.testConfig.testMode);
}

const char* CommandLineParser::getArgument(int argn)  const
{
    return (argn < m_argc) ? m_argv[argn] : nullptr;
}

const char* CommandLineParser::getParam(int argn, const char* option, const char* param)
{
    assert(option != nullptr);

    const char* const paramStr = getArgument(argn);
    if (paramStr == nullptr)
        throw CommandLineException("Missing <%s> parameter for \"%s\"", param, option);

    return paramStr;
}

const char* CommandLineParser::getParamAsStr(int argn, const char* option, const char* param)
{
    return getParam(argn, option, param);
}

long CommandLineParser::getParamAsLong(int argn, const char* option, const char* param)
{
    const char* const paramStr = getParam(argn, option, param);
    
    char* endptr = nullptr;
    errno = 0;
    const long value = ::strtol(paramStr, &endptr, 0);
    if (errno != 0)
        throw CommandLineException("Error converting parameter %s <%s> = '%s' to long",
            option, param, paramStr);
    else if (endptr != nullptr && *endptr != '\0')
        throw CommandLineException("Parameter value %s <%s> = '%s' contains illegal trailing characters: %s",
            option, param, paramStr, endptr);
    
    return value;
}

unsigned long CommandLineParser::getParamAsULong(int argn, const char* option, const char* param)
{
    const char* const paramStr = getParam(argn, option, param);
    
    char* endptr = nullptr;
    errno = 0;
    const unsigned long value = ::strtoul(paramStr, &endptr, 0);
    if (errno != 0)
        throw CommandLineException("Error converting parameter %s <%s> = '%s' to unsigned long",
            option, param, paramStr);
    else if (endptr != nullptr && *endptr != '\0')
        throw CommandLineException("Parameter value %s <%s> = '%s' contains illegal trailing characters: %s",
            option, param, paramStr, endptr);

    return value;
}

int CommandLineParser::getParamAsInt(int argn, const char* option, const char* param)
{
    return static_cast<int>(getParamAsLong(argn, option, param));
}

unsigned int CommandLineParser::getParamAsUInt(int argn, const char* option, const char* param)
{
    return static_cast<unsigned int>(getParamAsULong(argn, option, param));
}

uint8_t CommandLineParser::getParamAsUInt8(int argn, const char* option, const char* param)
{
    unsigned long tmpValue = getParamAsULong(argn, option, param);
    if (tmpValue > static_cast<uint8_t>(~0))
        throw CommandLineException("Parameter value %s <%s> = %lu is out of range",
            option, param, tmpValue);

    return static_cast<uint8_t>(tmpValue);
}

uint16_t CommandLineParser::getParamAsUInt16(int argn, const char* option, const char* param)
{
    unsigned long tmpValue = getParamAsULong(argn, option, param);
    if (tmpValue > static_cast<uint16_t>(~0))
        throw CommandLineException("Parameter value %s <%s> = %lu is out of range",
            option, param, tmpValue);

    return static_cast<uint16_t>(tmpValue);
}

uint32_t CommandLineParser::getParamAsUInt32(int argn, const char* option, const char* param)
{
    return getParamAsUInt(argn, option, param);
}

double CommandLineParser::getParamAsDouble(int argn, const char* option, const char* param)
{
    const char* const paramStr = getParam(argn, option, param);

    char* endptr = nullptr;
    errno = 0;
    const double value = ::strtod(paramStr, &endptr);
    if (errno != 0)
        throw CommandLineException("Error converting parameter %s <%s> = '%s' to double",
            option, param, paramStr);
    else if (endptr != nullptr && *endptr != '\0')
        throw CommandLineException("Parameter value %s <%s> = '%s' contains illegal trailing characters: %s",
            option, param, paramStr, endptr);

    return value;
}

bool CommandLineParser::optionEqual(const char* option, const char* test)
{
    assert(option != nullptr);
    assert(test != nullptr);
    
    // Check that option is started with "--" or "-" and, if so,
    // remove the prefix. Otherwise report a mismatch (return false).
    if (::strncmp(option, "--", 2) == 0)
        option += 2;
    else if (::strncmp(option, "-", 1) == 0)
        option += 1;
    else
        return false;
    
    return (::strcmp(option, test) == 0);
}

char* CommandLineParser::uint32ToBinStr(uint32_t value, char* binStr, size_t size)
{
    const size_t nbits = sizeof(value) * 8;
    if (nbits + 1 > size)
        throw ApplicationException("uint32ToBinStr() failed due to insufficient buffer size");
    
    for (size_t i = 0; i < nbits; ++i)
    {
        binStr[i] = (value & 1 ? '1' : '0');
        value >>= 1;
    }
    
    binStr[nbits] = '\0';
    return binStr;
}

uint32_t CommandLineParser::binStrToUInt32(const char* binStr)
{
    assert(binStr != nullptr);
    
    uint32_t result = 0;

    const size_t len = ::strlen(binStr);
    for (size_t i = 0; i < len; i++)
    {
        result <<= 1;

        switch (binStr[i])
        {
        case '0':
            result += 0;
            break;
        case '1':
            result += 1;
            break;
        default:
            throw CommandLineException("Cannot convert binary string to int: %s", binStr);
        }
    }
    
    return result;
}
