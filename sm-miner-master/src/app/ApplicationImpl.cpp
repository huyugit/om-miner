/*
 * Contains ApplicationImpl class definition.
 */

#include "ApplicationImpl.h"
#include "app/Application.h"
#include "base/BaseUtil.h"
#include "base/PollTimer.h"
#include "sys/SingleInstanceApp.h"
#include "sys/writer/StreamWriter.h"
#include "except/ApplicationException.h"
#include "config/CommandLineParser.h"
#include "hw/CpuInfo.h"
#include "hw/GpioManager.h"
#include "env/EnvManager.h"
#include "slave-gate/SlaveGate.h"
#include "stats/MasterStat.h"
#include "stats/json/JsonStat.h"
#include "board_revisions.h"
#include "web-gate/webgate.h"
#include "sys-monitor-gate/sys_monitor_gate.h"
#include "stats/FanStat.h"
#include "log/LogManager.h"
#include "log/LogCategories.h"
#include "test/ServerTest.h"
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>


// Anonymous namespace.
namespace
{
    // Create application logger.
    Logger& logger = LogManager::instance().createLogger(LogCategories::c_appLogCtg);
    
}  // End of anonymous namespace.


ApplicationImpl::ApplicationImpl()
    : m_appRegistry()
    , m_initialized(false)
    , m_started(false)
    , m_needExit(false)
    , m_returnCode(EXIT_SUCCESS)
    , m_config()
    , m_configMutex()
    , m_eventManager(m_appRegistry)
    , m_gpioPolling(m_appRegistry)
    , m_stratumPool(m_appRegistry)
{
}

ApplicationImpl::~ApplicationImpl() throw()
{
    // Ensure the application is uninitialized.
    done();
}

void ApplicationImpl::onConfigChange(Config& config)
{
    LOG_TRACE(logger) << "Processing configuration change...\n";
	LOG_INFO(logger) << "Processing configuration change...\n";

    ensureInitialized();
    
    // Analyze configuration change and determine a set of actions to process.
    const Config oldConfig = *this->config();
    const ConfigActions actions = analyzeConfigChange(config, oldConfig);
    
    // Check if the new configuration is different from the previous one (oldConfig).
    if (!actions)
    {
        LOG_TRACE(logger) << "No configuration changes to process.\n";
		LOG_INFO(logger) << "No configuration changes to process.\n";
        return;
    }

    // Check if the restart is needed to accommodate config change.
    if (actions.isSet(ConfigActions::EXIT_APP))
    {
        LOG_WARN(logger) << "Application needs to restart because of the configuration change, exiting...\n";
		LOG_INFO(logger) << "Application needs to restart because of the configuration change, exiting...\n";
        requestExit(EXIT_SUCCESS);
        return;
    }
    
    // Update the application config.
    updateConfig(config);
    
    // Process remaining config actions.
    processConfigActions(actions, config, oldConfig);
    
    LOG_TRACE(logger) << "Configuration change handled.\n";
	LOG_INFO(logger) << "Configuration change handled.\n";
}

const Config& ApplicationImpl::defaults() throw()
{
    // Define a static Config instance holding the configuration default values.
    static const Config s_defaults;
    return s_defaults;
}

const LockedConfig ApplicationImpl::config()
{
    return LockedConfig(m_config, m_configMutex);
}

Config &ApplicationImpl::configRW()
{
    return m_config;
}

void ApplicationImpl::parseCommandLine(int argc, const char* argv[])
{
    // Create new configuration object as a copy of the current application config.
    Config config = *this->config();
    
    // Update configuration according to the command line.
    CommandLineParser::parse(argc, argv, config);
    
    // Update the application config.
    updateConfig(config);
}

void ApplicationImpl::configPostProcessing()
{
}

void ApplicationImpl::init()
{
    if (m_initialized)
        throw ApplicationException("Application already initialized.");
    
    const Config config = *this->config();
    
    // Configure logging.
    configureLogging(config);

    LOG_TRACE(logger) << "Initializing application...\n";

    SingleInstanceApp::lock("/var/run/sm-miner.pid");

    g_masterStat.init();

    LOG_INFO(logger) << "Init gpio\n";
    g_cpuInfo.init();
    g_gpioManager.init();

    LOG_INFO(logger) << "Init env\n";
    g_envManager.init();

    LOG_INFO(logger) << "Init slave MCU\n";
    g_slaveGate.init();

    // Call init() on all registered application components.
    if (m_appRegistry.selectComponents())
    {
        while (AppComponent* appComponent = m_appRegistry.nextComponent())
            appComponent->init(config);
    }
    
    m_initialized = true;

    LOG_TRACE(logger) << "Application initialized.\n";
}

void ApplicationImpl::run()
{
    LOG_TRACE(logger) << "Running the application...\n";

    ensureInitialized();

    if (config()->testConfig.testMode != TEST_MODE_NONE)
    {
        g_serverTest.run();
    }
    else
    {
        // Process production mode run.
        start();
        appMain();
        stop();
    }
}

void ApplicationImpl::done() throw()
{
    if (!m_initialized)
        return;
    
    // Ensure the application is stopped.
    stop();
    
    LOG_TRACE(logger) << "Uninitializing the application...\n";
    
    // Call "done()" on all application components in reverse order
    // (last registered to be disposed first).
    if (m_appRegistry.selectComponents(true))
    {
        while (AppComponent* appComponent = m_appRegistry.nextComponent())
            appComponent->done();
    }
    
    m_initialized = false;
    
    LOG_TRACE(logger) << "Application uninitialized.\n";
}

void ApplicationImpl::requestExit(int rc)
{
    LOG_TRACE(logger) << "Scheduling application exit...\n";
    
    m_needExit = true;
    m_returnCode = rc;
}

// Starts the application.
void ApplicationImpl::start()
{
    LOG_TRACE(logger) << "Starting application components...\n";
    
    if (m_started)
        throw ApplicationException("Application is already started.");
    
    ensureInitialized();
    
    // Start all the application components.
    if (m_appRegistry.selectComponents())
    {
        while (AppComponent* appComponent = m_appRegistry.nextComponent())
            appComponent->start();
    }
    
    m_started = true;
}

// Stops the application.
void ApplicationImpl::stop()
{
    if (!m_started)
        return;  // Already stopped.
    
    LOG_TRACE(logger) << "Stopping application components...\n";
    
    // Call "stop()" on all application components in reverse order
    // (last registered to be stopped first).
    if (m_appRegistry.selectComponents(true))
    {
        while (AppComponent* appComponent = m_appRegistry.nextComponent())
            appComponent->stop();
    }
    
    m_started = false;
}

int ApplicationImpl::checkFanSpeedThreshold(uint8_t temp)
{
	if (temp <= HSB_TEMP_LOW) {
		return FAN_SPEED_BASE;
	} else {
		return (FAN_SPEED_BASE + ((temp - HSB_TEMP_LOW) * FAN_CTL_STEP));
	}
}

// Runs the application main loop.
void ApplicationImpl::appMain()
{
	int HSBTempMax = 0;
	
    LOG_INFO(logger) << "Enter main cycle.\n";
    PollTimer statTimer;
	PollTimer fanStatTimer;
    PollTimer minerAliveEventTimer;
	PollTimer minerReady;
	PollTimer fanFixRpmTimer;
	bool minerStateReady = false;
    m_eventManager.reportEvent(EventType::MINER_START, "Enjoy your mining!");

	//chenbo add begin 20171220
	SysMonitorGate minersysmonitorgate;
	//chenbo add end

	minerReady.start();

    while (!m_needExit)
    {
        Config cfg = *config().getConfig();

        g_slaveGate.runPollingIteration();
        g_envManager.runPollingIteration();

		if(g_webgate.psuWorkCond.FanFlag) {
			if (minerStateReady) {
				if (g_webgate.psuWorkCond.FanWorkMode == AUTO_SPEED_MODE) {
					HSBTempMax = g_envManager.getTempMax();
					
					if (HSBTempMax >= HSB_TEMP_LOW && HSBTempMax <= HSB_TEMP_HIGH) {
						g_fanstat.setFanPWM(PWM_CTL_BASE + ((HSBTempMax - HSB_TEMP_LOW) * PWM_CTL_STEP));
					} else if (HSBTempMax > HSB_TEMP_HIGH) {
						g_fanstat.setFanPWM(PWM_FULL_SPEED);
					} else {
						g_fanstat.setFanPWM(PWM_CTL_BASE);
					}
				} else if (g_webgate.psuWorkCond.FanWorkMode == FAN_FULL_SPEED) {
					g_fanstat.setFanPWM(PWM_FULL_SPEED);
				} else {
					printf("Invaild fan ctl mode found\n");
				}
			} else {
				g_fanstat.setFanPWM(PWM_FULL_SPEED);
				/* waiting 5min for miner state ready, sync with slave fix fan speed control issue */
				if (minerReady.isElapsedSec(MINER_READY_TIME)) {
					minerReady.stop();
					minerStateReady = true;
					printf("#####miner status ready#####\n");
				}
			}
		}else{
			/* if set fix RPM in /tmp/json/fan-speed-config.json */
			if(fanFixRpmTimer.isElapsedSec(5)){
				fanFixRpmTimer.start();
				g_webgate.PullFanRpmConfig();	//gzh add to update fix fan RPM config 20180507
				for(uint32_t fanId = 0; fanId < FAN_NUM; fanId++){
					uint32_t fanRpm = g_fanstat.getFanRPMSpeed(fanId);
					uint32_t fanPwm = g_fanstat.getFanPWM(fanId);
					if(fanRpm > g_webgate.fanRpmCfg[fanId]){
						uint32_t pwmDiff = (fanRpm - g_webgate.fanRpmCfg[fanId])/100;
						if(pwmDiff > 0){
							g_fanstat.setFanPWM(fanPwm - pwmDiff);
						}
					}else{
						uint32_t pwmDiff = (g_webgate.fanRpmCfg[fanId] - fanRpm)/100;
						if(pwmDiff > 0){
							g_fanstat.setFanPWM(fanPwm + pwmDiff);
						}
					}
				}
			}
		}

		if(fanStatTimer.isElapsedMs(50)){
			fanStatTimer.start();
			/* update fan status json file */
			g_fanstat.setFanRPMSpeed();
			g_fanstat.setFanFaultStat();
			g_fanstat.setFanPWMValue();
			g_webgate.PushMinerStatus(MINER_FAN_STATUS_MASK);
			/* update the sysmonitor.json */
			minersysmonitorgate.SysMonitorRunPollingIteration();
    	}
	
        if (statTimer.isElapsedSec(cfg.logDelaySec))
        {
            statTimer.start();

            // Collect the iteration statistics and aggregate.
            g_masterStat.aggregate();

            StreamWriter wr(stdout);
            g_masterStat.printStat(wr);

			//chenbo add begin 20171127
			g_webgate.PushMinerStatus(MINER_STATUS_ALL);

			g_webgate.GetMinerConfig(cfg);
			Config& webminerconf = g_webgate.minerconfig;
			onConfigChange(webminerconf);
			//chenbo add end

            g_masterStat.saveStat();
        }

        if (cfg.aliveEventIntervalSec > 0 &&
            minerAliveEventTimer.isElapsedSec(cfg.aliveEventIntervalSec))
        {
            m_eventManager.reportEvent(EventType::MINER_ALIVE, "Enjoy your mining!");
            minerAliveEventTimer.start();
        }

        ::usleep(1000 * cfg.pollingDelayMs);
    }

    LOG_TRACE(logger) << "Leaving main cycle...\n";
}

// Check that the application has been initialized and throw an exception if it has not.
void ApplicationImpl::ensureInitialized()
{
    if (!m_initialized)
        throw ApplicationException("Application has not been yet initialized.");
}

// Configure/reconfigure log levels.
void ApplicationImpl::configureLogging(const Config& config)
{
    const bool traceAll = config.traceAll;
    
    LogManager::instance()
        .configure(Logger::traceOrInfo(traceAll))
        .configure(LogCategories::c_appLogCtg, Logger::traceOrInfo(traceAll))
        .configure(LogCategories::c_configLogCtg, Logger::traceOrInfo(traceAll))
        .configure(LogCategories::c_gpioLogCtg, Logger::traceOrInfo(traceAll))
        .configure(LogCategories::c_lcdLogCtg, Logger::traceOrInfo(traceAll))
        .configure(LogCategories::c_stratumLogCtg, Logger::traceOrInfo(traceAll || config.traceStratum))
        .configure(LogCategories::c_eventLogCtg, Logger::traceOrInfo(traceAll))
        ;
    
    LOG_INFO(logger) << "Global tracing is " << (traceAll ? "ON" : "OFF") << ".\n";
}

// Update the application config.
void ApplicationImpl::updateConfig(const Config& config)
{
    // Update the application config data object.
    Mutex::Lock lock(m_configMutex);
    m_config = config;
    lock.unlock();
}

ApplicationImpl::ConfigActions::ConfigActions()
{
    for (size_t i = 0; i < util::arrayLength(m_actionFlags); ++i)
        m_actionFlags[i] = false;
}

bool ApplicationImpl::ConfigActions::operator!() const throw()
{
    for (size_t i = 0; i < util::arrayLength(m_actionFlags); ++i)
    {
        if (m_actionFlags[i])
            return false;
    }
    
    return true;
}

void ApplicationImpl::ConfigActions::set(Type value)
{
    const size_t index = static_cast<size_t>(value);
    assert(index < util::arrayLength(m_actionFlags));
    m_actionFlags[index] = true;
}

bool ApplicationImpl::ConfigActions::isSet(Type value) const
{
    const size_t index = static_cast<size_t>(value);
    assert(index < util::arrayLength(m_actionFlags));
    return m_actionFlags[index];
}

const char* ApplicationImpl::ConfigActions::typeToString(Type value) throw()
{
    switch (value)
    {
        case NONE:                  return "NONE";
        case AUTO:                  return "AUTO";
        case EXIT_APP:              return "EXIT_APP";
        case UPDATE_LOG_CFG:        return "UPDATE_LOG_CFG";
        case UPDATE_POOL_CFG:       return "UPDATE_POOL_CFG";
        case RESTART_POOL:          return "RESTART_POOL";
        default: ;
    }
    
    return "UNKNOWN";
}

// Analyzes config file change and determines the action to be taken to process.
const ApplicationImpl::ConfigActions ApplicationImpl::analyzeConfigChange(
    const Config& config, const Config& oldConfig)
{
	ConfigActions CA;

	if((config.poolConfig.host != oldConfig.poolConfig.host)
		||(config.poolConfig.port != oldConfig.poolConfig.port)
		||(config.poolConfig.userName != oldConfig.poolConfig.userName)
		#ifdef MULTI_POOL_SUPPORT
		// fcj add begin 20180313
		// bak1 pool
		||(config.poolConfig.bak1Host != oldConfig.poolConfig.bak1Host)
		||(config.poolConfig.bak1Port != oldConfig.poolConfig.bak1Port)
		||(config.poolConfig.bak1UserName != oldConfig.poolConfig.bak1UserName)
		// bak2 pool
		||(config.poolConfig.bak2Host != oldConfig.poolConfig.bak2Host)
		||(config.poolConfig.bak2Port != oldConfig.poolConfig.bak2Port)
		||(config.poolConfig.bak2UserName != oldConfig.poolConfig.bak2UserName)
		// fcj add end
		#endif
    	)
	{
		CA.set(ConfigActions::UPDATE_POOL_CFG);
		CA.set(ConfigActions::RESTART_POOL);
	}

	return CA;
}

// Does the actual processing of the configuration change according to the given actions.
void ApplicationImpl::processConfigActions(const ConfigActions& actions,
    Config& config, const Config& oldConfig)
{
    // Update logging settings.
    if (actions.isSet(ConfigActions::UPDATE_LOG_CFG))
    {
        LOG_INFO(logger) << "Logging settings changed.\n";
        configureLogging(config);
    }

    // Update pool settings.
    if (actions.isSet(ConfigActions::UPDATE_POOL_CFG)
      | actions.isSet(ConfigActions::RESTART_POOL))
    {
        m_stratumPool.reconfigure(config);
    }

    // Restart pool.
    if (actions.isSet(ConfigActions::RESTART_POOL))
    {
        LOG_INFO(logger) << "Pool will be restarted because of the config change...\n";
        m_stratumPool.restart();
    }
}
