/*
 * Contains StratumPool class definition.
 */

#include "StratumPool.h"

#include "app/Application.h"
#include "env/EnvManager.h"
#include "env/GpioPolling.h"

#include "pool/ShareValidator.h"

#include "events/EventManager.h"

#include "log/LogManager.h"
#include "log/LogCategories.h"
#include "log/BflLog.h"

#include "sys/NetworkSocket.h"

#include "except/SystemException.h"
#include "except/ApplicationException.h"

#include "base/MiscUtil.h"
#include "base/VarArgs.h"

#include "old/cJSonVar.h"

#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <error.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>


// Anonymous namespace.
namespace
{
    // Create Stratum logger.
    Logger& logger = LogManager::instance().createLogger(LogCategories::c_stratumLogCtg);
    
    // Input/output buffer size for receiving/sending pool data 
    const size_t c_inputBufferSize = 8 * 1024;  // 8 Kb
    const size_t c_outputBufferSize = 4 * 1024;  // 4 Kb
    
    // A number of seconds between adjacent reconnect attempts.
    const unsigned int c_reconnectDelaySec = 5;

    // A number of seconds to wait before retrying to establish a new session
    // after the previous session set-up failure, such as authorization or
    // subscription error.
    const unsigned int c_sessionRetryDelaySec = 5;

    // A maximum number of seconds of non-receiving new jobs.
    // If elapsed, connection to Pool is reset.
    const unsigned int c_noNewJobsTimeoutSec = 30*60;
    
    // A maximum number of seconds since the last acceptance of a submitted share.
    // If elapsed, connection to Pool is reset.
    const unsigned int c_shareNonacceptanceTimeoutSec = 30*60;

    // A maximum number of successively rejected shares.
    // If reached, all current jobs and pending shares are cleaned up.
    const unsigned int c_maxSuccessiveRejects = 32;


    // Define exception class to report Pool connection failures.
    DEFINE_APP_EXCEPTION(ConnectionException);

    // Define exception class to report Pool session set-up failures
    // such as authorization or subscription rejects.
    DEFINE_APP_EXCEPTION(SessionSetupException);

    const bool c_stratumDebugJob = false;

}  // End of anonymous namespace.


StratumPool::StratumPool(AppRegistry& appRegistry)
    : AppComponent(appRegistry)
    //
    , m_poolConfig()
    , m_newPoolConfig()
    , m_configChanged(false)
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180314
    , m_currentPoolInfoCount(0)
    , m_checkDefaultPoolInfoTimer()
    // fcj add end
#endif
    , m_inputBuffer(c_inputBufferSize)
    , m_outputBuffer(c_outputBufferSize)
    , m_protocolState(ProtocolState::OFFLINE)
    , m_stateTimer()
    , m_connectedTimer(false)
    , m_noNewJobTimer(false)
    , m_shareNonacceptanceTimer(false)
    , m_subscribeResendTimer(false)
    //, m_controlPipe()
    , m_netThread()
    , m_lastRequestId(0)
    , m_subscribeRequestId(0)
    , m_authorizeRequestId(0)
    , m_jobsMutex()
    , m_sharesMutex()
    , m_jobs()
    , m_shares()
    , m_jobIdCounter(c_firstJobIdCounter)
    , m_extraNonce1Size(0)
    , m_extraNonce2Size(0)
    , m_difficulty(0)
    , m_difficultyReceived(false)
    , m_connectionAttempts(0)
    , m_successiveRejects(0)
    //
    , m_statMutex()
    , m_totalPoolTimer(false)
    , m_inServiceTimer(false)
    , m_receivedJobs(0)
    , m_receivedJobsWithClean(0)
    , m_sentShares(0)
    , m_acceptedShares(0)
    , m_rejectedShares(0)
    , m_acceptedSolutions(0)
{
    memset(m_extraNonce1, 0, sizeof(m_extraNonce1));
#ifdef MULTI_POOL_SUPPORT    
    // fcj add begin 20180314
    // PoolInfo array init
    memset(m_poolInfoArray, 0, sizeof(struct PoolInfo)*3);
    // fcj add end
#endif
}

StratumPool::~StratumPool() throw()
{
    done();  // Ensure the object is uninitialized properly.
}

#ifdef MULTI_POOL_SUPPORT
// fcj add begin 20180314
void StratumPool::poolConfigToPoolInfoArray()
{
	// default pool
	m_poolInfoArray[0].isEnabled = true;
	m_poolInfoArray[0].host = m_poolConfig.host;
	m_poolInfoArray[0].port = m_poolConfig.port;
	m_poolInfoArray[0].userName = m_poolConfig.userName;
	m_poolInfoArray[0].password = m_poolConfig.password;
	m_poolInfoArray[0].retryCount = 0;
	
	// bak1 pool
	m_poolInfoArray[1].isEnabled = false;
	if (!m_poolConfig.bak1Host.isEmpty()
		&& !m_poolConfig.bak1Port.isEmpty()
		&& !m_poolConfig.bak1UserName.isEmpty())
	{
		m_poolInfoArray[1].isEnabled = true;
		m_poolInfoArray[1].host = m_poolConfig.bak1Host;
		m_poolInfoArray[1].port = m_poolConfig.bak1Port;
		m_poolInfoArray[1].userName = m_poolConfig.bak1UserName;
		m_poolInfoArray[1].password = m_poolConfig.bak1Password;
		m_poolInfoArray[1].retryCount = 0;
	}

	// bak2 pool
	m_poolInfoArray[2].isEnabled = false;
	if (!m_poolConfig.bak2Host.isEmpty()
		&& !m_poolConfig.bak2Port.isEmpty()
		&& !m_poolConfig.bak2UserName.isEmpty())
	{
		m_poolInfoArray[2].isEnabled = true;
		m_poolInfoArray[2].host = m_poolConfig.bak2Host;
		m_poolInfoArray[2].port = m_poolConfig.bak2Port;
		m_poolInfoArray[2].userName = m_poolConfig.bak2UserName;
		m_poolInfoArray[2].password = m_poolConfig.bak2Password;
		m_poolInfoArray[2].retryCount = 0;
	}

	// Default use default pool
	m_currentPoolInfoCount = 0;
}
// fcj add end

// fcj add begin 20180314
// Re-select PoolInfo
void StratumPool::reselectPoolInfo() {
	//Maximum count of retries not reached
	if (m_poolInfoArray[m_currentPoolInfoCount].retryCount < m_poolConfig.retryNTimes)
	{
		m_poolInfoArray[m_currentPoolInfoCount].retryCount++;
		LOG_INFO(logger) << "### Pool[" << m_currentPoolInfoCount << "] retry " \
			<< m_poolInfoArray[m_currentPoolInfoCount].retryCount << " times, But not reselect...\n";
		return;
	}

	//Maximum count of retries reached, Reset and modify
	m_poolInfoArray[m_currentPoolInfoCount].retryCount = 0;
	while (1)
	{
		m_currentPoolInfoCount++;
		if (m_currentPoolInfoCount >= 3)
		{
			m_currentPoolInfoCount = 0;
		}
		if (m_poolInfoArray[m_currentPoolInfoCount].isEnabled)
		{
			break;
		}
	}
	
	LOG_INFO(logger) << "### Reselect to Pool[" << m_currentPoolInfoCount << "]...\n";
	if (m_currentPoolInfoCount > 0)
	{
		m_checkDefaultPoolInfoTimer.reset();
		m_checkDefaultPoolInfoTimer.start();
	}
	return;
}
// fcj add end

// fcj add begin 20180314
// Check default PoolInfo, true: trun to default, false: do nothing
bool StratumPool::checkDefaultPoolInfo()
{
	// Default PoolInfo in use, do nothing
	if (m_currentPoolInfoCount == 0)
	{
		return false;
	}

	// Other PoolInfo in use, but no timeout(10s), do nothing
	if (!m_checkDefaultPoolInfoTimer.isElapsedSec(10))
	{
		return false;
	}
	
	NetworkSocket defaultNetSock;
	try
	{
		defaultNetSock.connect(getDefaultPoolInfo().host.cdata(), getDefaultPoolInfo().port.cdata());
	}
	catch (const Exception& e)
	{
		m_checkDefaultPoolInfoTimer.reset();
		m_checkDefaultPoolInfoTimer.start();
		LOG_INFO(logger) << "### Check default pool, but also down...\n";
		return false;
	}

	if (!defaultNetSock.isClosed()) {
		defaultNetSock.close();
	}

	LOG_INFO(logger) << "### Default pool alive, reselect default pool...\n";
	m_poolInfoArray[m_currentPoolInfoCount].retryCount = 0;
	m_currentPoolInfoCount = 0;
	return true;
}
// fcj add end

// fcj add begin 20180319
// Retry successful, Resetting retry information
void StratumPool::resetCurrentPoolRetryInfo()
{
	if (m_poolInfoArray[m_currentPoolInfoCount].retryCount > 0)
	{
		LOG_INFO(logger) << "### Pool[" << m_currentPoolInfoCount << "] alive, Resetting retry information...\n";
		m_poolInfoArray[m_currentPoolInfoCount].retryCount = 0;
	}
}
// fcj add end
#endif

void StratumPool::doInit(const Config& config)
{
    LOG_TRACE(logger) << "Initializing Pool...\n";

    m_poolConfig = config.poolConfig;
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180314
    // m_poolConfig -> m_poolInfoArray[3]
    poolConfigToPoolInfoArray();
    // fcj add end
#endif    
    m_totalPoolTimer.start();
}

void StratumPool::doStart()
{
    LOG_INFO(logger) << "Starting Stratum Pool...\n";

    // fcj modify begin 20180314
#ifdef MULTI_POOL_SUPPORT
    LOG_INFO(logger) << "Pool: " << getCurrentPoolInfo().host.cdata() << ":" << getCurrentPoolInfo().port.cdata()
        << " (user " << getCurrentPoolInfo().userName.cdata() << ")\n\n";
    // fcj modify end
#else
    LOG_INFO(logger) << "Pool: " << m_poolConfig.host.cdata() << ":" << m_poolConfig.port.cdata()
        << " (user " << m_poolConfig.userName.cdata() << ")\n\n";
#endif
    try
    {
        if (Application::config()->testConfig.testMode == TEST_MODE_NONE)
        {
            m_netThread.start(this);
        }
    }
    catch (const Exception& e)
    {
    	gStratumLog.writeLog(START_ERR);
        throw ApplicationException("Unable to start Stratum Pool thread: %s", e.what());
    }
}

void StratumPool::doStop() throw()
{
    LOG_TRACE(logger) << "Stopping Pool...\n";
	
    m_netThread.cancel(true);
    m_netThread.join();
}

void StratumPool::doDone() throw()
{
    LOG_TRACE(logger) << "Uninitializing Pool...\n";
    
    m_totalPoolTimer.stop();
}

void StratumPool::reconfigure(const Config& config)
{
    LOG_TRACE(logger) << "Updating Pool configuration...\n";

    // When the pool is running, pool configuration m_poolConfig
    // is accessed only from the network thread. To apply the change,
    // new config data is copied into intermediate m_newPoolConfig
    // variable and m_configChanged flag is set signaling the
    // network thread to pick up the updates.
    m_newPoolConfig = config.poolConfig;
    m_configChanged = true;
    
    notifyNetThread();
}

unsigned int StratumPool::getBootstrapStatus() throw()
{
    unsigned int status = 0;
    
    Mutex::Lock jobsLock(m_jobsMutex);
    if (m_jobs.isEmpty())
        status |= BootstrapFlags::PENDING_JOB;
    jobsLock.unlock();

    if (!m_difficultyReceived)
        status |= BootstrapFlags::PENDING_DIFFICULTY;

    return status;
}

bool StratumPool::getCurrentJob(StratumJob& job, uint32_t lastJobId /*= 0*/)
{
    Mutex::Lock lock(m_jobsMutex);

    if (m_jobs.getSize() == 0)
        return false;

    const StratumJob& lastJob  = m_jobs.tail();
    if (lastJob.getJobId() == lastJobId)
        return false;

    job = lastJob;
    return true;
}

bool StratumPool::getJob(uint32_t jobId, StratumJob &job)
{
    Mutex::Lock lock(m_jobsMutex);

    for (size_t i = 0; i < m_jobs.getSize(); ++i)
    {
        if (m_jobs[i].getJobId() == jobId)
        {
            job = m_jobs[i];
            return true;
        }
    }

    return false;
}

void StratumPool::submitShare(const StratumShare& share)
{
    Mutex::Lock lock(m_sharesMutex);
    m_shares.enqueue(share);
    lock.unlock();
    
    // New shares may not be sent too Pool until the session is up
    // (e.g., after some disconnect).
    if (m_protocolState == ProtocolState::IN_SERVICE)
        notifyNetThread();
}

// Apply new pool configuration if it is available.
// Needs to be invoked from the pool network thread.
// Returns true if the configuration has been updated and false otherwise.
bool StratumPool::checkNewConfig()
{
    if (!m_configChanged)
        return false;
    
    // Update the current pool configuration.
    m_poolConfig = m_newPoolConfig;
    m_configChanged = false;

#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180314
    // m_poolConfig -> m_poolInfoArray[3]
    poolConfigToPoolInfoArray();
    // fcj add end
#endif
    LOG_INFO(logger) << "Pool configuration has been updated.\n";
    return true;
}

// Updates the protocol state with the given value.
void StratumPool::setState(ProtocolState::Type value) throw()
{
    m_protocolState = value;
    m_stateTimer.start();
}

// Checks if all bootstrap data (job, difficulty, etc.) are ready
// and, if yes, brings Pool into In-service state.
void StratumPool::processBootstrap()
{
    if (getBootstrapStatus() != 0)
    {
        LOG_TRACE(logger) << "More bootstrap data expected...\n";
        return;
    }
    
    // Going In-service....
    setState(ProtocolState::IN_SERVICE);
    
    Mutex::Lock statLock(m_statMutex);
    m_inServiceTimer.resume();
    statLock.unlock();
    
    m_noNewJobTimer.start();  // Reset 'no new job' timeout.
    
    LOG_INFO(logger) << "Pool in-service, we can send shares!\n";
    REPORT_EVENT(EventType::POOL_INFO, "Pool in-service");
}

// A root method handling network communication.
// Executed in a separate thread.
void StratumPool::netHandler()
{
    LOG_TRACE(logger) << "Entered Stratum pool network processing thread 0x"
        << Logger::format("%08lx", m_netThread.getThreadId()) << ".\n";
    
    LOG_INFO(logger) << "Stratum Pool started.\n" << Logger::flush();
    
    setState(ProtocolState::OFFLINE);
    
    m_connectionAttempts = 0;
    
    // A number of seconds to wait before retrying to reconnect.
    unsigned int postDelaySec = 0;
    
    while (!m_netThread.isCancelRequested())
    {
        if (postDelaySec > 0)
        {
            LOG_INFO(logger) << "Will retry in " << postDelaySec << " seconds...\n";
            ::sleep(postDelaySec);
            postDelaySec = 0;
        }
        
        bool errorOccurred = true;
        
        try
        {
            processNetSession();
            errorOccurred = false;
        }
        catch (const ConnectionException& e)
        {
            LOG_ERROR(logger) << "Pool connection failed: " << e.what() << "\n";
            REPORT_EVENT(EventType::POOL_ERROR, "ConnectionException: unable connect to stratum server.");
            gStratumLog.writeLog(CONNECT_ERR);
            if (m_connectionAttempts >= m_poolConfig.maxConnectionAttempts)
            {
                LOG_ERROR(logger) << "Couldn't connect to the pool after "
                    << m_connectionAttempts<< " attempts!\n"
                    << "Exiting...\n" << Logger::flush();
                REPORT_EVENT(EventType::POOL_ERROR,
                    "Pool connection attempts exceed the limit, exiting...");
                
                Application::requestExit(EXIT_FAILURE);
                break;
            }
            
            postDelaySec = c_reconnectDelaySec;
		#ifdef MULTI_POOL_SUPPORT
            // fcj add begin 20180314
            // Re-select PoolInfo
            reselectPoolInfo();
            // fcj add end
        #endif
        }
        catch (const SessionSetupException& e)
        {
        	gStratumLog.writeLog(SESSION_ERR);
            LOG_ERROR(logger) << e.what() << "\n";
            REPORT_EVENT(EventType::POOL_ERROR, "SessionSetupException: %s", e.what());
            
            postDelaySec = c_sessionRetryDelaySec;
		#ifdef MULTI_POOL_SUPPORT
            // fcj add begin 20180314
            // Re-select PoolInfo
            reselectPoolInfo();
            // fcj add end
        #endif
        }
        catch (const Exception& e)
        {
	        gStratumLog.writeLog(EXCEPTION_ERR);
            LOG_ERROR(logger) << e.what() << "\n";
            REPORT_EVENT(EventType::POOL_ERROR, "Exception: %s", e.what());

            postDelaySec = c_reconnectDelaySec;
		#ifdef MULTI_POOL_SUPPORT
            // fcj add begin 20180314
            // Re-select PoolInfo
            reselectPoolInfo();
            // fcj add end
        #endif
        }

        if (postDelaySec <= 0)
        {
            postDelaySec = c_reconnectDelaySec;
        }
        
        if (m_protocolState >= ProtocolState::CONNECTED)
        {
            LOG_INFO_IF(logger, m_protocolState >= ProtocolState::CONNECTED)
                << "Disconnected from pool.\n" << Logger::flush();
            REPORT_EVENT(EventType::RECONNECTION);
        }
        
        bool needExit= false;
        if (errorOccurred && m_protocolState == ProtocolState::IN_SERVICE)
        {
            REPORT_EVENT(EventType::RECONNECTION_ON_ERROR);
            if (m_poolConfig.exitOnError)
                needExit= true;
        }
        
        setState(ProtocolState::OFFLINE);
        
        Mutex::Lock statLock(m_statMutex);
        m_inServiceTimer.stop();
        statLock.unlock();
        
        m_inputBuffer.clear();
        
        if (needExit)
        {
            LOG_WARN(logger) << "Exiting (in exit-on-error mode)...\n" << Logger::flush();
            Application::requestExit(EXIT_FAILURE);
            break;
        }
    }
    
    LOG_TRACE(logger) << "Leaving Stratum pool network thread.\n";
}

// Method handling network communication session (from "connect" to "close").
void StratumPool::processNetSession()
{
    // Share acceptance timer is started/restarted each time a positive
    // submit response is received. Before receiving a first acknowledge,
    // the timer is stopped.
    m_shareNonacceptanceTimer.stop();
    
    // Clean up from previous session jobs/shares.
    clearQueues();
    
    // Check for configuration updates.
    checkNewConfig();
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180319
    // Check default PoolInfo
    checkDefaultPoolInfo();
    // fcj add end
#endif
    // Init statistic counters.
    m_successiveRejects = 0;
    
    // Reset JSON-RPC request counters.
    m_lastRequestId = 0;
    m_subscribeRequestId = 0;
    m_authorizeRequestId = 0;
    
    // Set difficulty to default value.
    m_difficulty = m_poolConfig.initialDifficulty;
    m_difficultyReceived = false;
    LOG_INFO(logger) << "Initial difficulty: " << m_difficulty << "\n";
    
    NetworkSocket netSock;
    
    connectToPool(netSock);
    //suggestDifficulty(netSock);	//add suggest difficulty by gezhihua 20180220
    subscribeToPool(netSock);
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180319
    // Check if retry successful, Resetting retry information
    resetCurrentPoolRetryInfo();
    // fcj add end
#endif    
    // Define an array of polling requests for arriving data
    // from the control pipe and pool network socket.
    pollfd pollReq[1];
    
    // pollReq indexes.
    static const int c_pollSocket = 0;
//    static const int c_pollControl = 1;

    // Enter socket send/receive loop.
    while (!netSock.isClosed() && !m_netThread.isCancelRequested())
    {
        // Add the network socket descriptor into the poll request list.
        pollReq[c_pollSocket].fd = netSock.getDescriptor();
        pollReq[c_pollSocket].events = POLLIN;

//        pollReq[c_pollControl].fd = m_controlPipe.getSock2().getDescriptor();
//        pollReq[c_pollControl].events = POLLIN;

        static const int c_pollTimeoutMs = 200;
        if (::poll(pollReq, util::arrayLength(pollReq), c_pollTimeoutMs) < 0)
        {
            if (errno != EINTR)
            {
                LOG_ERROR(logger) << "StratumPool: poll() failed: "
                    << UnixErrorDescription(errno).cstr() << "\n";
            }
            
            ::sleep(0);
            continue;
        }
        
        // Check for configuration updates.
        checkNewConfig();
#ifdef MULTI_POOL_SUPPORT
		// fcj add begin 20180314
		// Check default PoolInfo
		if (checkDefaultPoolInfo())
		{
			netSock.close();
			break;
		}
		// fcj add end
#endif
        // If there is a notification data send to the control pipe,
        // consume it from the pipe buffer.
//        if (pollReq[c_pollControl].revents & POLLIN)
//            consumeNotificationData();
        
        // Check if the data from Pool is available.
        if (pollReq[c_pollSocket].revents & POLLIN)
            onInputData(netSock);

        if (!m_subscribeResendTimer.isStopped() && m_subscribeResendTimer.isElapsedSec(5))
        {
            m_subscribeResendTimer.stop();
            subscribeToPool(netSock);
        }
        
        // Send pending shares (if any) to Pool.
        // No share is sent to Pool until the authorization is done
        // and the Pool state becomes "in-service".
        if (hasSharesToSend() && m_protocolState == ProtocolState::IN_SERVICE)
            sendAllPendingShares(netSock);
        
        // Event checks.
        checkAuthorizationTimeout();
        checkSubscriptionTimeout();
        checkBootstrapTimeout();
        checkNoNewJobTimeout();
        checkNonacceptanceTimeout();
    }
}

// Establishes network connection with the Pool.
void StratumPool::connectToPool(NetworkSocket& netSock)
{
    // fcj modify begin 20180314
    #ifdef MULTI_POOL_SUPPORT
	LOG_INFO(logger) << "Connecting to " << getCurrentPoolInfo().host.cdata() << ":"
			<< getCurrentPoolInfo().port.cdata() << "...\n" << Logger::flush();
	#else
    LOG_INFO(logger) << "Connecting to " << m_poolConfig.host.cdata() << ":"
        << m_poolConfig.port.cdata() << "...\n" << Logger::flush();
    // fcj modify end
    #endif
    setState(ProtocolState::CONNECTING);
    
    try
    {
        ++m_connectionAttempts;
	// fcj modify begin 20180314
        #ifdef MULTI_POOL_SUPPORT
        netSock.connect(getCurrentPoolInfo().host.cdata(), getCurrentPoolInfo().port.cdata());
		#else
		netSock.connect(m_poolConfig.host.cdata(), m_poolConfig.port.cdata());
		#endif
	// fcj modify end
        m_connectionAttempts = 0;
    }
    catch (Exception& e)
    {
    	gStratumLog.writeLog(CONNECT_ERR);
        throw ConnectionException("%s", e.what());
    }
    
    setState(ProtocolState::CONNECTED);
    m_connectedTimer.start();
    
    LOG_INFO(logger) << "Connection successful\n";
}

// Sends subscription request to Pool.
void StratumPool::subscribeToPool(NetworkSocket& netSock)
{
	bool addSubscribeParam = false;
    LOG_TRACE(logger) << "Subscribing to mining job notifications...\n";
#ifdef MULTI_POOL_SUPPORT
	/*
		For btcc pool, subscribe message length should longer than 53 bytes, 
		the original message length is 48, so add some data in parameters.
	*/
	if(strstr(getCurrentPoolInfo().host.cdata(), "btcc")){
		addSubscribeParam = true;
	}
#else
	if(strstr(m_poolConfig.host.cdata(), "btcc")){
		addSubscribeParam = true;
	}
#endif
	
	if(addSubscribeParam){
		static const char* const c_format = "{\"id\":%d,\"method\":\"mining.subscribe\",\"params\":[\"%s\",%d]}\n";
		setState(ProtocolState::SUBSCRIPTION);
		m_subscribeRequestId = getNextRequestId();
		formatOutput(c_format, m_subscribeRequestId, "cgminer4.9", 8);
	}else{
    	static const char* const c_format = "{\"id\":%d,\"method\":\"mining.subscribe\",\"params\":[]}\n";
		setState(ProtocolState::SUBSCRIPTION);
		m_subscribeRequestId = getNextRequestId();
		formatOutput(c_format, m_subscribeRequestId);
	}

    sendOutputData(netSock);
}

// Sends worker authorization request to Pool.
void StratumPool::authorizeWorker(NetworkSocket& netSock)
{
    LOG_TRACE(logger) << "Authorizing worker...\n";

    static const char* const c_format =
        "{\"id\":%d,\"method\":\"mining.authorize\",\"params\":[\"%s\",\"%s\"]}\n";
    
    setState(ProtocolState::AUTHORIZATION);
    m_authorizeRequestId = getNextRequestId();
    formatOutput(c_format, m_authorizeRequestId,
	// fcj modify begin 20180319
        #ifdef MULTI_POOL_SUPPORT
        getCurrentPoolInfo().userName.cdata(), getCurrentPoolInfo().password.cdata());
		#else
		m_poolConfig.userName.cdata(), m_poolConfig.password.cdata());
		#endif
	// fcj modify end
    sendOutputData(netSock);
}

// Sends suggest difficulty to pool
void StratumPool::suggestDifficulty(NetworkSocket& netSock)
{
    LOG_TRACE(logger) << "Suggest Difficulty...\n";

    static const char* const c_format =
        "{\"id\":%d,\"method\":\"mining.suggest_difficulty\",\"params\":[%d]}\n";
    
    setState(ProtocolState::SUGGESTDIFF);
    m_suggestDifficultyRequestId = getNextRequestId();
    formatOutput(c_format, m_suggestDifficultyRequestId, 65536);
    sendOutputData(netSock);
}

// Generates new JSON-RPC request ID.
int StratumPool::getNextRequestId()
{
    if (++m_lastRequestId < 0)
        m_lastRequestId = 1;

    return m_lastRequestId;
}

// Prepare Stratum request based on the given format and arguments.
// format - format string, same as for printf().
void StratumPool::formatOutput(const char* format, ...)
{
    VarArgs arglist;
    VA_START(arglist, format);
    const size_t nchars = ::vsnprintf(m_outputBuffer.cdata(), m_outputBuffer.getCapacity(),
        format, arglist);
    
    if (nchars >= m_outputBuffer.getCapacity())
        throw ApplicationException("Not enough buffer capacity to format Stratum message.");
    m_outputBuffer.resize(nchars);
}

// Writes data from the output buffer into the given socket.
void StratumPool::sendOutputData(NetworkSocket& netSock)
{
    uint8_t* data = m_outputBuffer;
    size_t len = m_outputBuffer.getSize();
    
    if (logger.admitsTrace())
    {
        if (len < m_outputBuffer.getCapacity() && data[len] != 0)
            data[len] = 0;
        
        if (data[len] == 0)
            LOG(logger) << "Stratum sending: " << m_outputBuffer.cdata() << Logger::flush();
        else
            LOG(logger) << "Stratum sending: ... (" << len << " bytes)\n" << Logger::flush();
    }
    
    // We need a loop for sending, because not all of the data may be written
    // in one call; send() will return how many bytes were sent.
    while (len > 0)
    {
        const size_t bytesSent = netSock.send(data, len);
        data += bytesSent;
        len -= bytesSent;
    }
    
    m_outputBuffer.clear();
}

// Reads and interprets input data from the given socket.
void StratumPool::onInputData(NetworkSocket& netSock)
{
    if (m_inputBuffer.getAvailableCapacity() == 0)
    {
        LOG_WARN(logger) << "Input buffer overflow, clearing...\n";
        m_inputBuffer.clear();
    }

    size_t sizeBefore = m_inputBuffer.getSize();
    const size_t bytesReceived = netSock.receive(m_inputBuffer.data() + sizeBefore,
        m_inputBuffer.getCapacity() - sizeBefore);
    if (bytesReceived == 0)
        return;

    size_t sizeAfter = sizeBefore + bytesReceived;
    m_inputBuffer.resize(sizeAfter);
    
    for (;;)
    {
        // Scan input buffer for the end of command line.
        size_t eolPos = sizeBefore;
        for (; eolPos < sizeAfter; ++eolPos)
        {
            if (m_inputBuffer[eolPos] == '\n')
                break;
        }
        
        if (eolPos >= sizeAfter)
            break;  // End of command line not found, to be received.

        // Replace the end of command line (\n) character with 0
        // to be able to treat the buffer content as C-string.
        m_inputBuffer[eolPos] = 0;
        
        // Parse message from Pool and initiate the corresponded processing.
        onInputMessage(netSock, m_inputBuffer.cdata(), eolPos);
        
        // Shift the buffer data to sweep the interpreted message string.
        m_inputBuffer.shiftLeft(eolPos + 1);
        
        sizeBefore = 0;
        sizeAfter -= eolPos + 1;
    }
}

// Parses and process input Stratum message from Pool.
void StratumPool::onInputMessage(NetworkSocket& netSock,
    const char* messageStr, size_t messageLen)
{
    LOG_TRACE(logger) << "Stratum received (len = " << messageLen << "): " << messageStr << "\n"
        << Logger::flush();

    char messageForParsing[2048];
    if (messageLen+1 > sizeof(messageForParsing))
    {
        LOG_WARN(logger) << "Too big notify from server, length: " << messageLen << "\n";
        return;
    }

    memcpy(messageForParsing, messageStr, messageLen+1);

    const cJSonVar* const head = ParseJSon(messageForParsing);
    if (!head || !head->IsHash())
        return;  // JSON parsing error.
    
    try
    {
        const cJSonVar* const idVar = (*head)["id"];
        if (!idVar || idVar->IsNull())  // Notification from Pool.
        {
            onNotifyFromPool(head);
        }
        else  // Request or response from Pool. Requests are not supported.
        {
            if (!idVar->IsInteger())
                throw ApplicationException("\"id\" must be integer or null in JSON string.");
            const int requestId = idVar->GetIntValue();
            
            if (!(*head)["result"] && !(*head)["error"])
            {
                const cJSonVar* const methodVar = (*head)["method"];

                if (::strcmp(methodVar->GetValue(), "client.get_version") == 0)
                {
                    LOG_WARN(logger) << "Unsupported notify: " << methodVar->GetValue() << "\n";
                }
                else
                {
                    LOG_WARN(logger) << "JSON-RPC request from Pool - discarding...\n";
                    LOG_INFO(logger) << "Discarded request data (len = " << messageLen << "): " << messageStr << "\n";
                    //TODO: problem with above code - messageStr is destroyed after call to ParseJSon(messageStr)!
                }
            }
            else
            {
                if (requestId == m_authorizeRequestId)
                {
                    onResponseToAuthorize(netSock, head);
                }
                else if (requestId == m_subscribeRequestId)
                {
                    onResponseToSubscribe(netSock, head);
                }
                else
                {
                    onResponseToSubmit(netSock, head, messageStr);
                }
            }
        }
    }
    catch (...)
    {
    	gStratumLog.writeLog(INPUTMSG_ERR);
        LOG_WARN(logger) << "Error processing input Stratum message:\n";
        head->PrintSelf(stdout);
        ::fflush(stdout);
        delete head;
        throw;
    }
    
    delete head;
}

void StratumPool::onResponseToSubscribe(NetworkSocket& netSock, const cJSonVar* head)
{
    LOG_TRACE(logger) << "Processing \"mining.subscribe\" response...\n";
    
    if (c_stratumDebugJob)
    {
        // Replace pool's messages with test data
        static char messageStr[128];
        strcpy(messageStr, "{\"id\":1,\"result\":[[[\"mining.set_difficulty\",\"b4b6693b72a50c7116db18d6497cac52\"],[\"mining.notify\",\"ae6812eb4cd7735a302a8a9dd95cf71f\"]],\"927dcb18\",4],\"error\":null}\n");

        const cJSonVar* const newHead = ParseJSon(messageStr);
        if (!newHead || !newHead->IsHash())
        {
        	gStratumLog.writeLog(SUBSCRIBE_ERR);
            return;  // JSON parsing error.
        }

        head = newHead;
    }

    if (m_protocolState != ProtocolState::SUBSCRIPTION)
    {
    	gStratumLog.writeLog(SUBSCRIBE_ERR);
        LOG_WARN(logger) << "Not in subscription state, \"mining.subscribe\" response discarded.\n";
        return;
    }
    
    const cJSonVar* const resultVar = (*head)["result"];
    if (resultVar && resultVar->GetValue() && strcmp(resultVar->GetValue(), "Initialising") == 0) {
		gStratumLog.writeLog(SUBSCRIBE_ERR);
        // ckpool Initialising message
        LOG_WARN(logger) << "Initialising \"result\" received.\n";
        REPORT_EVENT(EventType::SUBSCRIBE_ERROR);
        m_subscribeResendTimer.start();
        return;
    }

    if (!resultVar || resultVar->IsNull() || !resultVar->IsArray()) {
		gStratumLog.writeLog(SUBSCRIBE_ERR);
        throw ApplicationException("Invalid or missing \"result\" member.");
        return;
    }
    
    onNewExtranonce((*resultVar)[1], (*resultVar)[2]);

    authorizeWorker(netSock);
}

void StratumPool::onResponseToAuthorize(NetworkSocket& netSock, const cJSonVar* head)
{
    LOG_TRACE(logger) << "Processing \"mining.authorize\" response...\n";
    
    if (m_protocolState != ProtocolState::AUTHORIZATION)
    {
        LOG_WARN(logger) << "Not in authorization state, \"mining.authorize\" response discarded.\n";
        return;
    }

    // Determine if there is an "error" member in the response.
    bool hasError = false;
    const cJSonVar* const errorVar = (*head)["error"];
    if (errorVar && !errorVar->IsNull())
    {
        hasError = true;
        LOG_TRACE(logger) << "Negative \"mining.authorize\" response received:\n";
        head->PrintSelf(stdout);
    }

    // Determine authorization decision.
    const cJSonVar* const resultVar = (*head)["result"];
    const bool authorized = (resultVar && resultVar->IsBool()) ?
        resultVar->GetBoolValue() : !hasError;

    if (!authorized)
        throw SessionSetupException("Pool authorization failed, verify login/password.");

    LOG_INFO(logger) << "Authorization succeeded.\n";
    
    setState(ProtocolState::BOOTSTRAP);
    processBootstrap();
}

void StratumPool::onResponseToSubmit(NetworkSocket& netSock, const cJSonVar* head, const char* messageStr)
{
    LOG_TRACE(logger) << "Processing \"mining.submit\" response...\n";

    const cJSonVar* const idVar = (*head)["id"];
    assert(idVar != nullptr && !idVar->IsNull() && idVar->IsInteger());
    const int requestId = idVar->GetIntValue();

    // Possible cases:
    // {"id": 2, "result": true, error=NULL} <-- normal case
    // {"id": 6, "error": (23, "Low difficulty share", null)}  <-- ghash.io (result is missing)
    // {"id": 9, "result": null, "error": (21, "Job not found", null)} <-- bitcoin.cz

    // Determine if there is an "error" member in the response.
    bool hasError = false;
    const cJSonVar* const errorVar = (*head)["error"];
    if (errorVar && !errorVar->IsNull())
    {
    	//gStratumLog.writeLog(SUBMIT_ERR);
        hasError = true;
        LOG_TRACE(logger) << "Negative \"mining.submit\" response received:\n";
        head->PrintSelf(stdout);
    }

    // Determine the acceptance status.
    const cJSonVar* const resultVar = (*head)["result"];
    const bool accepted = (resultVar && resultVar->IsBool()) ?
        resultVar->GetBoolValue() : !hasError;

    if (accepted)
    {
        LOG_TRACE(logger) << "Share accepted " << requestId << "!\n";

        lockedStatInc(m_acceptedShares);
        lockedStatInc(m_acceptedSolutions, m_difficulty);

        m_successiveRejects = 0;
        
        // Reset share acceptance timeout.
        m_shareNonacceptanceTimer.start();
    }
    else
    {
        LOG_WARN(logger) << "Share NOT accepted " << requestId << "!\n";

        StratumShare share;
        int reqAge = -1;

        char shareStr[512] = "N/A";
        char jobStr[4096] = "N/A";

        if (m_reqMgr.getShare(requestId, share, reqAge))
        {
            share.toString(shareStr, sizeof(shareStr));

            StratumJob job;
            if (getJob(share.getJobId(), job))
            {
                job.toString(jobStr, sizeof(jobStr));
            }
        }

        REPORT_EVENT(EventType::POOL_SHARE_DISCARDED, "POOL response: %s, REQ: id=%d, age=%d, SHARE: %s, JOB: %s",
                     messageStr, requestId, reqAge, shareStr, jobStr);

        lockedStatInc(m_rejectedShares);
        ++m_successiveRejects;
        
        if (m_successiveRejects > c_maxSuccessiveRejects)
        {
            // A series of successive rejects from Pool usually means that
            // something is wrong with the current round of calculations.
            // Try clearing queues first to avoid massive rejects situation.
            // If doesn't help, m_shareNonacceptanceTimer will cause reconnection.
            
            LOG_WARN(logger) << "Too many successive rejects from pool, clearing queues...\n";
            clearQueues();
            
            // Reset successive reject counter.
            m_successiveRejects = 0;
        }
    }

    m_reqMgr.remove(requestId);
}

void StratumPool::onNotifyFromPool(const cJSonVar* head)
{
    LOG_TRACE(logger) << "Processing notification from pool...\n";
    
    const cJSonVar* const methodVar = (*head)["method"];
    if (!methodVar || !methodVar->IsVariable())
        throw ApplicationException("Invalid or missing \"method\" in the notification message.");

    if (::strcmp(methodVar->GetValue(), "client.get_version") == 0)
    {
        onClientGetVersion(head);
    }
    else if (::strcmp(methodVar->GetValue(), "client.show_message") == 0)
    {
        onClientShowMessage(head);
    }
    else if(strcmp(methodVar->GetValue(), "mining.set_extranonce")==0)
    {
        onMiningSetExtranonce(head);
    }
    else if (::strcmp(methodVar->GetValue(), "mining.notify") == 0)
    {
        onNewMiningJob(head);
    }
    else if (::strcmp(methodVar->GetValue(), "mining.set_difficulty") == 0)
    {
        onSetDifficulty(head);
    }
    else
    {
        LOG_WARN(logger) << "Unknown notify (" << methodVar->GetValue() << "):\n";
        head->PrintSelf(stdout);
        ::fflush(stdout);
    }
}

void StratumPool::onClientGetVersion(const cJSonVar *head)
{
    LOG_TRACE(logger) << "Processing \"client.get_version\" notification...\n";
    head->PrintSelf(stdout);
}

void StratumPool::onClientShowMessage(const cJSonVar *head)
{
    // {params=(1)["You've connected to Mintsy!"],method="client.show_message",id=NULL}

    LOG_TRACE(logger) << "Processing \"client.show_message\" notification...\n";
    head->PrintSelf(stdout);
}

void StratumPool::onMiningSetExtranonce(const cJSonVar *head)
{
    // {params=(2)["f80093ec",4],method="mining.set_extranonce",id=NULL}

    LOG_TRACE(logger) << "Processing \"client.set_extranonce\" notification...\n";
    head->PrintSelf(stdout);

    const cJSonVar *params=(*head)["params"];
    if (params) {
        onNewExtranonce(params->GetByIndex(0), params->GetByIndex(1));
    }
    else {
        throw ApplicationException("Invalid or missing \"params\" in \"mining.set_extranonce\" message.");
    }
}

void StratumPool::onNewExtranonce(const cJSonVar *valExtraNonce1, const cJSonVar *valExtraNonce2Size)
{
    // Extract Extranonce1 parameter.
    if (!valExtraNonce1 || !valExtraNonce1->IsVariable())
        throw ApplicationException("Invalid or missing \"Extranonce1\" parameter.");

    size_t en1Chars = strlen(valExtraNonce1->GetValue());
    if (en1Chars % 2 != 0)
        throw ApplicationException("Invalid \"Extranonce1\" - length is not pow of 2.");

    m_extraNonce1Size = en1Chars / 2;

    if (m_extraNonce1Size > c_extraNoncesMaxSize)
        throw ApplicationException("Invalid \"Extranonce1\" - length is too big.");

    util::hexToData(valExtraNonce1->GetValue(), m_extraNonce1, en1Chars);

    // Extract Extranonce2_size parameter.
    if (!valExtraNonce2Size || !valExtraNonce2Size->IsInteger())
        throw ApplicationException("Invalid or missing \"Extranonce2_size\" parameter.");

    m_extraNonce2Size = valExtraNonce2Size->GetIntValue();

    if (m_extraNonce2Size > c_extraNoncesMaxSize)
        throw ApplicationException("We do not currently support ExtraNonce2 longer then %zu bytes.",
        c_extraNoncesMaxSize);

    if (logger.admitsInfo())
    {
        logger.printf("Server setup for us extranonce1: "); logger.hexdump(m_extraNonce1, m_extraNonce1Size);
        logger.printf("Server setup for us size of extranonce2: %u\n", m_extraNonce2Size);
    }

    clearQueues();
}

void StratumPool::onSetDifficulty(const cJSonVar* head)
{
    LOG_TRACE(logger) << "Processing difficulty change...\n";

    if (c_stratumDebugJob)
    {
        // Replace pool's messages with test data
        static char messageStr[128];
        strcpy(messageStr, "{\"id\":null,\"params\":[1],\"method\":\"mining.set_difficulty\"}\n");

        const cJSonVar* const newHead = ParseJSon(messageStr);
        if (!newHead || !newHead->IsHash())
            return;  // JSON parsing error.

        head = newHead;
    }

    const cJSonVar* const paramsVar = (*head)["params"];
    if (!paramsVar || !paramsVar->IsArray())
	{
		gStratumLog.writeLog(SETDIFF_ERR);
        throw ApplicationException("Invalid or missing \"params\" in \"mining.set_difficulty\" message.");
    }
    const cJSonVar* const difficultyVar = paramsVar->GetByIndex(0);
    const int difficulty = difficultyVar->GetIntValue();
    if (difficulty <= 0)
    {
    	gStratumLog.writeLog(SETINVDIFF_ERR);
        LOG_WARN(logger) << "Zero or negative difficulty from Pool, ignored.\n";
        return;
    }
    
    m_difficultyReceived = true;
    
    if (static_cast<uint32_t>(difficulty) != m_difficulty)
    {
        const uint32_t oldDifficulty = m_difficulty;
        m_difficulty  = difficulty;
        
        LOG_INFO(logger) << "Pool changed difficulty to " << difficulty << "\n";
        REPORT_EVENT(EventType::DIFF_CHANGE, "Difficulty changed: %u => %u",
            oldDifficulty, m_difficulty);
    }
    else
    {
        LOG_TRACE(logger) << "No change to the current difficulty.\n";
    }
    
    if (m_protocolState == ProtocolState::BOOTSTRAP)
        processBootstrap();
}

void StratumPool::onNewMiningJob(const cJSonVar* head)
{
    LOG_TRACE(logger) << "Processing new mining job...\n";

    if (c_stratumDebugJob)
    {
        // Replace pool's messages with test data
        static char messageStr[2*1024];
        strcpy(messageStr, "{\"id\":null,\"params\":[\"18cb89d4\",\"ff593a402266a4f65913d3904027f656c3ddf2da0c39c1970000000000000000\",\"01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4803313f05062f503253482f043201e35408\",\"2e522cfabe6d6d6e26ab6fb7fca0b9e4299a7dde345cccc3dfe03cc3f62599048137d4e3b090d00400000000000000000000000146bade95000000001976a91480ad90d403581fa3bf46086a91b2d9d4125db6c188ac00000000\",[\"4757fee28d8b141b089c18326be7c0f1f35d4e486ceef19dc6cfa216c35f3528\",\"fbbb905cb2e8c48050baeca763c32a6eb873d2108195208524af2eed22c64c5a\",\"95411c30c95713cc5f7980e802dc42e292695b38b7e1d9907ba247dd7efb4d7c\",\"392f9f03b8647fdd8fe6ec30edc5dc0327adfa6977346b199cbdfd90f3fb72d7\",\"98b6e7ff87db70a9de871dad663a92ffebc84ef88e4339ef15ef8a547d6013fd\",\"515beb3e9fe3b47218ea4f38ddcae0b1071718569445e50cc9d421aa74171ce8\",\"d518563e68317b3789b074387e6945030dd0504bc019a59dca6897e30b626426\",\"31edcee5993dc066eb93c61f28f76eadf61fd691bdbcdcb415096c1409f227d8\",\"e074da8beb298a15ce1df1b640d76e3e805a63f926f74e30ea84f54d87caf25c\",\"4b941922dc95080cb31b272e74656c5f15f7276f33e63c58ddfff5636189abe8\",\"c9c096a4c18b9e7faeebde7fbbee1c13c051036bf0cdb63686f16f042fe5cc52\"],"
               "\"00000002\",\"1818bb87\",\"54e30130\",false],\"method\":\"mining.notify\"}\n");

        const cJSonVar* const newHead = ParseJSon(messageStr);
        if (!newHead || !newHead->IsHash())
            return;  // JSON parsing error.

        head = newHead;
    }

    const cJSonVar* const paramsVar = (*head)["params"];
    if (!paramsVar || !paramsVar->IsArray())
    {
    	gStratumLog.writeLog(NEWJOB_ERR);
        throw ApplicationException("Invalid or missing \"params\" in \"mining.notify\" message.");
    }
    
    // Indices of "mining.notify" parameters.
    enum
    {
        JOB_ID_PARAM = 0,       // ID of the job.
        PREVHASH_PARAM,         // Hash of previous block.
        COINB1_PARAM,           // Initial part of coinbase transaction.
        COINB2_PARAM,           // Final part of coinbase transaction.
        MERKLE_BRANCH_PARAM,    // List of hashes to calculate merkle root.
        VERSION_PARAM,          // Bitcoin block version.
        NBITS_PARAM,            // Encoded current network difficulty.
        NTIME_PARAM,            // Current ntime.
        CLEAN_JOBS_PARAM        // Whether to discard all unsent shares.
    };
    
    StratumJob newJob;
    
    const cJSonVar* paramVar = paramsVar->GetByIndex(JOB_ID_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"job_id\" parameter.");
    newJob.setJobName(paramVar->GetValue());
    
    // Set the Job timestamp as seconds since the Epoch.
    newJob.setTime(::time(nullptr));

    newJob.setDifficulty(m_difficulty);

    newJob.setExtraNonce1(m_extraNonce1, m_extraNonce1Size);

    newJob.setExtraNonce2Size(m_extraNonce2Size);

    // Extract "prevhash" parameter.
    paramVar = paramsVar->GetByIndex(PREVHASH_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"prevhash\" parameter.");
    if (util::hexToData(paramVar->GetValue(), newJob.getPrevHash(), c_sha256Size) != c_sha256Size)
        throw ApplicationException("Invalid length of \"prevhash\" parameter.");

    // Extract "coinb1" parameter.
    paramVar = paramsVar->GetByIndex(COINB1_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"coinb1\" parameter.");
    const size_t coinbase1Size = util::hexToData(paramVar->GetValue(),
        newJob.getCoinbase1(), c_coinbase1MaxSize);
    newJob.setCoinbase1Size(coinbase1Size);

    // Extract "coinb2" parameter.
    paramVar = paramsVar->GetByIndex(COINB2_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"coinb2\" parameter.");
    const size_t coinbase2Size = util::hexToData(paramVar->GetValue(),
        newJob.getCoinbase2(), c_coinbase2MaxSize);
    newJob.setCoinbase2Size(coinbase2Size);
    
    // Extract "merkle_branch" parameter.
    paramVar = paramsVar->GetByIndex(MERKLE_BRANCH_PARAM);
    if (!paramVar || !paramVar->IsArray())
        throw ApplicationException("Invalid or missing \"merkle_branch\" parameter.");
    
    size_t hashNum = 0;
    for (const cJSonVar* elementVar = paramVar->GetInternal();
        elementVar;
        elementVar = elementVar->GetNext(), ++hashNum)
    {
        if (!elementVar->IsVariable())
            throw ApplicationException("Invalid \"merkle_branch\"[%zu] element.", hashNum);
        
        uint8_t merkleHash[c_sha256Size];
        if (util::hexToData(elementVar->GetValue(), merkleHash, c_sha256Size) != c_sha256Size)
            throw ApplicationException("Invalid length of \"merkle_branch\"[%zu] element.", hashNum);
        
        newJob.addMerkleHash(merkleHash);
    }

    // Extract "version" parameter.
    paramVar = paramsVar->GetByIndex(VERSION_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"version\" parameter.");

    uint32_t tmpVersion = 0;
    if (util::hexToData(paramVar->GetValue(), &tmpVersion, 4) != 4)
        throw ApplicationException("Invalid length of \"version\" parameter.");
    newJob.setVersion(util::swapEndian(tmpVersion));

    // Extract "nbits" parameter.
    paramVar = paramsVar->GetByIndex(NBITS_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"nbits\" parameter.");

    uint32_t tmpNBits = 0;
    if (util::hexToData(paramVar->GetValue(), &tmpNBits, 4) != 4)
        throw ApplicationException("Invalid length of \"nbits\" parameter.");
    newJob.setNBits(util::swapEndian(tmpNBits));

    // Extract "ntime" parameter.
    paramVar = paramsVar->GetByIndex(NTIME_PARAM);
    if (!paramVar || !paramVar->IsVariable())
        throw ApplicationException("Invalid or missing \"ntime\" parameter.");

    uint32_t tmpNTime = 0;
    if (util::hexToData(paramVar->GetValue(), &tmpNTime, 4) != 4)
        throw ApplicationException("Invalid length of \"ntime\" parameter.");
    newJob.setNTime(util::swapEndian(tmpNTime));

    // Extract "clean_jobs" parameter.
    paramVar = paramsVar->GetByIndex(CLEAN_JOBS_PARAM);
    if (!paramVar || !paramVar->IsBool())
        throw ApplicationException("Invalid or missing \"clean_jobs\" parameter.");
    newJob.setCleanJobs(paramVar->GetBoolValue());
    
    queueJob(newJob);
    
    if (m_protocolState == ProtocolState::BOOTSTRAP)
        processBootstrap();
    else
        m_noNewJobTimer.start();  // Reset 'no new job' timeout.
}

const StratumStat StratumPool::cutOffStat()
{
    // Lock statistic data.
    Mutex::Lock statLock(m_statMutex);
    
    // Capture current statistics.
    const StratumStat resultStat(
        m_totalPoolTimer.elapsedMs(),
        m_inServiceTimer.elapsedMs(),
        m_receivedJobs,
        m_receivedJobsWithClean,
        m_sentShares,
        m_acceptedShares,
        m_rejectedShares,
        m_acceptedSolutions);
    
    // Reset statistic deltas.
    
    m_totalPoolTimer.start();
    
    m_inServiceTimer.reset();
    if (m_protocolState == ProtocolState::IN_SERVICE)
        m_inServiceTimer.start();
    
    m_receivedJobs = 0;
    m_receivedJobsWithClean = 0;
    
    m_sentShares = 0;
    m_acceptedShares = 0;
    m_rejectedShares = 0;
    m_acceptedSolutions = 0;
    
    return resultStat;
}

void StratumPool::logPoolStat(Writer &wr)
{
    // fcj modify begin 20180314
    #ifdef MULTI_POOL_SUPPORT
    wr.printf("Pool: host:port: %s:%s, user: %s, diff: %d\n",
              getPoolConfig().host.cdata(), getPoolConfig().port.cdata(),
              getPoolConfig().userName.cdata(), getCurrentDifficulty());
	#else
    wr.printf("Pool: host:port: %s:%s, user: %s, diff: %d\n",
              getPoolConfig().host.cdata(), getPoolConfig().port.cdata(),
              getPoolConfig().userName.cdata(), getCurrentDifficulty());
	#endif
    // fcj modify end

    char en1[c_extraNoncesMaxSize * 2 + 1];
    util::dataToHex(en1, m_extraNonce1, m_extraNonce1Size);

    wr.printf("extraNonce1: %s, extraNonce2Size: %u, jobs: %u\n",
              en1, m_extraNonce2Size, m_jobs.getSize());
}

// Adds the given job into the job queue assigning new Job ID.
void StratumPool::queueJob(StratumJob& newJob)
{
    lockedStatInc(m_receivedJobs);

//    REPORT_EVENT(EventType::POOL_JOB, "jobIdCounter = %u, job.name = %s, job.nTime = 0x%08x",
//                 m_jobIdCounter, newJob.getJobName(), newJob.getNTime());

    //if (newJob.getCleanJobs())
    /* to reduce the reject rate, clear all old jobs and calculate the last job */
    if(1)
    {
        lockedStatInc(m_receivedJobsWithClean);
        clearQueues();

        char jobStr[1024];
        newJob.toStringShort(jobStr, sizeof(jobStr));

        REPORT_EVENT(EventType::POOL_CLEAN_JOB, "jobIdCounter = %u, JOB: %s",
                     m_jobIdCounter, jobStr);
    }
    
    Mutex::Lock jobsLock(m_jobsMutex);
    
    // Check if the job with the same name already exists in the queue.
    const char* const jobName = newJob.getJobName();
    for (size_t i = 0; i < m_jobs.getSize(); ++i)
    {
        StratumJob& job = m_jobs[i];
        if (::strncmp(jobName, job.getJobName(), c_jobNameMaxLen + 1) == 0)
        {
            LOG_INFO(logger) << "Found existing job " << job.getJobId() << " [" << jobName << "]\n";
            return;
        }
    }
    
    // Assign new job ID.
    ++m_jobIdCounter;
    if (m_jobIdCounter < c_firstJobIdCounter)
        m_jobIdCounter = c_firstJobIdCounter;

    const uint32_t jobId = m_jobIdCounter;
    newJob.setJobId(jobId);

    if (c_stratumDebugJob)
    {
        newJob.setJobId(50);
    }
    
    // Enqueue the new job.
    m_jobs.enqueue(newJob);
    
    LOG_INFO(logger) << "Queued job " << jobId << " [" << jobName
        << "], job count = " << m_jobs.getSize() << "\n";
}

// Remove all active jobs and queued shares.
void StratumPool::clearQueues(bool solutionsOnly /*= false*/)
{
    if (!solutionsOnly)
        LOG_TRACE(logger) << "Clearing queues (" << m_jobs.getSize() << " jobs, "
            << m_shares.getSize() << " shares)\n";
    else
        LOG_TRACE(logger) << "Clearing solutions queue ("
            << m_shares.getSize() << " shares)\n";

    if (!solutionsOnly)
    {
        Mutex::Lock jobsLock(m_jobsMutex);
        m_jobs.clear();
    }

    Mutex::Lock sharesLock(m_sharesMutex);
    if (!m_shares.isEmpty())
    {
        REPORT_EVENT(EventType::POOL_ERROR, "Drop off %d shares", m_shares.getSize());
        m_shares.clear();
    }
}

// Notifies communication thread to awake.
// Upon receiving the notification, the network thread should check
// what kind of processing is needed.
void StratumPool::notifyNetThread()
{
    // Notification command to send through the control pipe.
    // Doesn't really matter what the actual value is.
//    static const char c_threadNotifyCommand = '!';

    // Send the notification command to the other control pipe endpoint.
//    m_controlPipe.getSock1().write(&c_threadNotifyCommand, 1);
}

// Reads all notification commands from the control pipe.
void StratumPool::consumeNotificationData()
{
    // To consume all data from the control pipe buffer, recv() function is used
    // instead of read(), since the latter blocks if there is no data available.

//    char command = 0;
//    while (::recv(m_controlPipe.getSock2().getDescriptor(), &command, 1, MSG_DONTWAIT) > 0)
//        ;  // Do nothing.
}

// Returns true if there are pending shares to send.
bool StratumPool::hasSharesToSend()
{
    Mutex::Lock lock(m_sharesMutex);
    return !m_shares.isEmpty();
}

// Sends all pending shares to Pool.
void StratumPool::sendAllPendingShares(NetworkSocket& netSock)
{
    Mutex::Lock lock(m_sharesMutex);
    
    StratumShare share;
    while (m_shares.dequeue(share))
    {
        lock.unlock();
        sendShare(netSock, share);
        lock.lock();
    }
}

// Sends the given share to Pool.
void StratumPool::sendShare(NetworkSocket& netSock, StratumShare& share)
{
    const uint32_t jobId = share.getJobId();
    
    char shareStr[512];
    share.toString(shareStr, sizeof(shareStr));

    LOG_TRACE(logger) << "Sending share for job " << jobId << "\n";
    
    if (jobId == 1)
    {
        REPORT_EVENT(EventType::DEFAULT_JOB_SHARE, "default job, SHARE: %s",
                     shareStr);
        return;
    }

    if (m_reqMgr.isDuplicate(share))
    {
        REPORT_EVENT(EventType::DUPLICATE_SHARE, "duplicate, SHARE: %s",
                     shareStr);
        return;
    }

    StratumJob job;
    if (!getJob(jobId, job))
    {
        REPORT_EVENT(EventType::STALE_JOB_SHARE, "job not found, SHARE: %s",
                     shareStr);
        return;
    }

    // HACK
    //share.shiftExtraNonce2(job.getExtraNonce2Size());   //chenbo delete for 8 Bytes extraNonce2 update bitfury 1205 code

    char jobStr[8*1024];
    job.toString(jobStr, sizeof(jobStr));

    const uint32_t jobDifficulty = job.getDifficulty();
    const uint32_t shareDifficulty = share.getDifficulty();
    
    if (shareDifficulty < jobDifficulty)
    {
        REPORT_EVENT(EventType::LOW_DIFF_SHARE, "shareDiff(%u) < jobDiff(%u), SHARE: %s",
                     shareDifficulty, jobDifficulty, shareStr);
        return;
    }

    {
        ShareValidator sv;
        sv.calculate(job, share);

        uint32_t actualDiff = sv.getShareDiff();

        if (actualDiff < jobDifficulty)
        {
            REPORT_EVENT(EventType::LOW_DIFF_SHARE, "actualDiff(%u) < jobDiff(%u),  SHARE: %s, JOB: %s",
                         actualDiff, jobDifficulty, shareStr, jobStr);
            return;
        }
    }

    char extraNonce2Str[c_extraNoncesMaxSize * 2 + 1];
    util::dataToHex(extraNonce2Str, share.getExtraNonce2Data(), job.getExtraNonce2Size());
    
    char nTimeStr[9];
    const uint32_t tmpNTime = util::swapEndian(share.getNTime());
    util::dataToHex(nTimeStr, &tmpNTime, 4);
    
    char nonceStr[9];
    const uint32_t tmpNonce = share.getNonce();
    util::dataToHex(nonceStr, &tmpNonce, 4);

    static const char* const c_format =
        "{\"id\":%d,\"method\":\"mining.submit\",\"params\":[\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"]}\n";

    const int requestId = getNextRequestId();
    // fcj modify begin 20180319
    #ifdef MULTI_POOL_SUPPORT
    formatOutput(c_format, requestId, getCurrentPoolInfo().userName.cdata(), job.getJobName(), extraNonce2Str, nTimeStr, nonceStr);
    #else
	formatOutput(c_format, requestId, m_poolConfig.userName.cdata(), job.getJobName(), extraNonce2Str, nTimeStr, nonceStr);
	#endif
    // fcj modify end

    m_reqMgr.shareSent(requestId, share);

    sendOutputData(netSock);
    lockedStatInc(m_sentShares);

//    REPORT_EVENT(EventType::SHARE_SENT, "SHARE: %s", shareStr);

    LOG_TRACE(logger) << "Sent new share " << requestId << " for job " << jobId
        << ", diff " << shareDifficulty << "\n";
}

// Check for authorization timeout.
void StratumPool::checkAuthorizationTimeout()
{
    if (m_protocolState == ProtocolState::AUTHORIZATION
        && m_stateTimer.isElapsedSec(m_poolConfig.authorizationTimeoutSec))
    {
    	gStratumLog.writeLog(AUTH_TO_ERR);
        throw ApplicationException("Authorization timed out.");
    }
}

// Check for subscription timeout.
void StratumPool::checkSubscriptionTimeout()
{
    if (m_protocolState == ProtocolState::SUBSCRIPTION
        && m_stateTimer.isElapsedSec(m_poolConfig.subscriptionTimeoutSec))
    {
    	gStratumLog.writeLog(SUBSB_TO_ERR);
        throw ApplicationException("Subscription timed out.");
    }
}

// Check for timeout receiving initial input data (difficulty, job, etc.).
void StratumPool::checkBootstrapTimeout()
{
    if (m_protocolState == ProtocolState::BOOTSTRAP
        && getBootstrapStatus() != 0
        && m_stateTimer.isElapsedSec(m_poolConfig.bootstrapTimeoutSec))
    {
        if (!m_difficultyReceived)
            throw ApplicationException("Timeout awaiting difficulty.");

        Mutex::Lock jobsLock(m_jobsMutex);
        if (!m_jobs.isEmpty())
            throw ApplicationException("Timeout awaiting initial mining job.");
        jobsLock.unlock();
        
        // Just a safeguard.
        throw ApplicationException("Bootstrap timeout.");
    }
}

// Check for new jobs non-receiving timeout.
void StratumPool::checkNoNewJobTimeout()
{
    if (m_protocolState == ProtocolState::IN_SERVICE
        && m_noNewJobTimer.isElapsedSec(c_noNewJobsTimeoutSec))
    {
    	gStratumLog.writeLog(NONEWJOB_TO_ERR);
        throw ApplicationException("No new job received during %u seconds.",
            c_noNewJobsTimeoutSec);
    }
}

// Check if the Pool keeps rejecting/ignoring our shares.
void StratumPool::checkNonacceptanceTimeout()
{
    if (m_protocolState == ProtocolState::IN_SERVICE
        && !m_shareNonacceptanceTimer.isStopped()
        && m_shareNonacceptanceTimer.isElapsedSec(c_shareNonacceptanceTimeoutSec))
    {
	    gStratumLog.writeLog(NOCACP_TO_ERR);
        throw ApplicationException("No solution accepted during %u seconds.",
            c_shareNonacceptanceTimeoutSec);
    }
}
