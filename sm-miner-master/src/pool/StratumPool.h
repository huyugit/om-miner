#ifndef STRATUM_POOL_H
#define STRATUM_POOL_H
/*
 * Contains StratumPool class declaration.
 */

#include "base/HeapBuffer.h"
#include "base/PollTimer.h"
#include "base/RingQueue.h"

#include "app/AppComponent.h"

#include "config/Config.h"

#include "pool/Stratum.h"
#include "pool/StratumJob.h"
#include "pool/StratumShare.h"
#include "pool/StratumStat.h"
#include "pool/StratumReqMgr.h"

#include "sys/ThreadRunner.h"
#include "sys/Mutex.h"
#include "sys/SocketPair.h"
#include "sys/writer/Writer.h"

#include <stdint.h>

#define SP_SET_NW_ERR_CODE(class, type, num)	((1 << 24) | ((class) << 16) | ((type) << 8) | (num))
#define START_ERR			0x01000001
#define CONNECT_ERR 		0x01000002
#define SESSION_ERR			0x01000003
#define EXCEPTION_ERR		0x01000004
#define INPUTMSG_ERR		0x01000005
#define SUBSCRIBE_ERR		0x01000006
#define SUBMIT_ERR			0x01000007
#define SETDIFF_ERR			0x01000008
#define SETINVDIFF_ERR		0x01000009
#define NEWJOB_ERR			0x0100000A
#define AUTH_TO_ERR			0x0100000B
#define SUBSB_TO_ERR		0x0100000C
#define NONEWJOB_TO_ERR		0x0100000D
#define NOCACP_TO_ERR		0x0100000E

#ifdef MULTI_POOL_SUPPORT
// fcj add begin 20180313
struct PoolInfo
{
	bool isEnabled; // is enabled
	StringBuffer<512> host;  // Host name.
	StringBuffer<128> port;  // Port.
	StringBuffer<128> userName;  // User name.
	StringBuffer<128> password;  // User password.
	int retryCount; // Retry count
};
// fcj add end
#endif

// Forward declarations.
class NetworkSocket;
class cJSonVar;

// Class to communicate with a mining pool via the Stratum protocol.
class StratumPool
    : protected AppComponent
{
friend class ApplicationImpl;

// Public type declarations.
public:
    // Stratum protocol states.
    struct ProtocolState
    {
        // When adding new type, also add LCD representation
        // to LcdManager::writePoolStateLine().
        enum Type
        {
            OFFLINE = 0,        // Not connected to Pool.
            CONNECTING,         // Connecting to Pool...
            CONNECTED,          // Network connection established.
            SUGGESTDIFF,		// Suggest difficulty by gezhihua 20180220
            SUBSCRIPTION,       // Awaiting subscription response.
            AUTHORIZATION,      // Awaiting authorization response.
            BOOTSTRAP,          // Awaiting difficulty and job.
            IN_SERVICE          // Normal work with the Pool.
        };
    };
    
    // A set of constant to test individual bit states in the bootstrap
    // status mask (see getBootstrapStatus() method).
    struct BootstrapFlags
    {
        static const unsigned int PENDING_DIFFICULTY = 0x1;
        static const unsigned int PENDING_JOB = 0x2;
    };

// Construction/destruction.
private:
    StratumPool(AppRegistry& appRegistry);
    ~StratumPool() throw();

// AppComponent implementation.
protected:
    virtual void doInit(const Config& config);
    virtual void doStart();
    virtual void doStop() throw();
    virtual void doDone() throw();

// Public interface.
public:
    // Updates Pool configuration.
    void reconfigure(const Config& config);
    
    // Returns the current protocol state.
    ProtocolState::Type getState() const throw()  { return m_protocolState; }
    
    // Returns a bit mask where individual bits correspond to bootstrap
    // process constituents (see BootstrapFlags).
    unsigned int getBootstrapStatus() throw();
    
    // Returns Pool configuration.
    const PoolConfig& getPoolConfig() throw()  { return m_poolConfig; }
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180314
    // Returns current PoolInfo.
    const PoolInfo& getCurrentPoolInfo() throw()  { return m_poolInfoArray[m_currentPoolInfoCount]; }
    // Returns default PoolInfo.
    const PoolInfo& getDefaultPoolInfo() throw()  { return m_poolInfoArray[0]; }
    // fcj add end
#endif
    // Returns the current Pool difficulty.
    inline uint32_t getCurrentDifficulty() const { return m_difficulty; }
    
    // Retrieves the latest job from Pool to process,
    // lastJobId - the previous job ID retrieved.
    // If the lastJobId differs from the ID of the latest job available,
    // returns true and copies the job data into the "job" argument,
    // otherwise returns false and no data is copied.
    bool getCurrentJob(StratumJob& job, uint32_t lastJobId = 0);

    bool getJob(uint32_t jobId, StratumJob& job);
    
    // Submits a found share to Pool.
    void submitShare(const StratumShare& share);
    
    // Records statistics checkpoint (cut-off).
    // Calculates and returns deltas since the previous cut-off.
    const StratumStat cutOffStat();

    void logPoolStat(Writer &wr);

// Implementation methods.
private:
    bool checkNewConfig();
    void setState(ProtocolState::Type value) throw();
    void processBootstrap();
    
    void netHandler();
    void processNetSession();
    
    void connectToPool(NetworkSocket& netSock);
	void suggestDifficulty(NetworkSocket& netSock);	//add by gezhihua for suggest difficulty 20180220
    void subscribeToPool(NetworkSocket& netSock);
    void authorizeWorker(NetworkSocket& netSock);
    
    int getNextRequestId();
    void formatOutput(const char* format, ...);
    void sendOutputData(NetworkSocket& netSock);
    
    void onInputData(NetworkSocket& netSock);
    void onInputMessage(NetworkSocket& netSock, const char *messageStr, size_t messageLen);
    
    // Methods to process input messages from Pool.
    void onResponseToSubscribe(NetworkSocket& netSock, const cJSonVar* head);
    void onResponseToAuthorize(NetworkSocket& netSock, const cJSonVar* head);
    void onResponseToSubmit(NetworkSocket& netSock, const cJSonVar* head, const char *messageStr);
    void onNotifyFromPool(const cJSonVar* head);
    void onClientGetVersion(const cJSonVar *head);
    void onClientShowMessage(const cJSonVar *head);
    void onMiningSetExtranonce(const cJSonVar *head);
    void onNewExtranonce(const cJSonVar *valExtraNonce1, const cJSonVar *valExtraNonce2Size);
    void onSetDifficulty(const cJSonVar* head);
    void onNewMiningJob(const cJSonVar* head);
    
    void queueJob(StratumJob& job);
    void clearQueues(bool solutionsOnly = false);
    
    void notifyNetThread();
    void consumeNotificationData();
    
    bool hasSharesToSend();
    void sendAllPendingShares(NetworkSocket& netSock);
    void sendShare(NetworkSocket& netSock, StratumShare &share);
    
    void checkAuthorizationTimeout();
    void checkSubscriptionTimeout();
    void checkBootstrapTimeout();
    void checkNoNewJobTimeout();
    void checkNonacceptanceTimeout();
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180314
    // m_poolConfig -> m_poolInfoArray[3]
    void poolConfigToPoolInfoArray();
    // Re-select PoolInfo
    void reselectPoolInfo();
    // Check default PoolInfo, true: trun to default, false: do nothing
    bool checkDefaultPoolInfo();
    // Retry successful, Resetting retry information
    void resetCurrentPoolRetryInfo();
    // fcj add end
#endif

    template<typename T>
    const T lockedStatInc(T& value, unsigned int incrementBy = 1)
    {
        Mutex::Lock statLock(m_statMutex);
        value += incrementBy;
        return value;
    }

// Member variables.
private:
    // Pool configuration settings.
    PoolConfig m_poolConfig;
    
    // Updated configuration settings pending to be picked up by the network thread.
    PoolConfig m_newPoolConfig;
    volatile bool m_configChanged;
#ifdef MULTI_POOL_SUPPORT
    // fcj add begin 20180314
    // PoolInfo array, 0:default pool, 1:bak1 pool, 2:bak2 pool
    PoolInfo m_poolInfoArray[3];
    // PoolInfo count in use
    int m_currentPoolInfoCount;
    // Timer to check default PoolInfo
    PollTimer m_checkDefaultPoolInfoTimer;
    // fcj add end
#endif
    // Input/output buffers for communication with the pool.
    HeapBuffer m_inputBuffer;
    HeapBuffer m_outputBuffer;
    
    // Stratum communication state 
    volatile ProtocolState::Type m_protocolState;
    
    // Timer started at the current state entry
    // to handle subscription and authorization timeouts.
    PollTimer m_stateTimer;
    
    // Timer measuring the interval of being connected to Pool.
    PollTimer m_connectedTimer;
    
    // Timer measuring a period of non-receiving new jobs.
    // After reaching a predefined limit, Pool connection will be reset.
    PollTimer m_noNewJobTimer;
    
    // Timer measuring a period of non-acceptance of submitted shares.
    // After reaching a predefined limit, Pool connection will be reset.
    PollTimer m_shareNonacceptanceTimer;

    PollTimer m_subscribeResendTimer;

    // A pair of connected sockets to signal network thread of new
    // solutions available to send.
    //SocketPair m_controlPipe;

    // The member object responsible for running data receiving thread.
    ThreadRunner<StratumPool, &StratumPool::netHandler> m_netThread;
    
    // Last Stratum JSON-RPC request ID.
    int m_lastRequestId;
    
    // JSON-RPC IDs of "subscribe" & "authorize" requests respectively.
    int m_subscribeRequestId;
    int m_authorizeRequestId;

	// add SON-RPC IDs of suggest difficulty by gezhihua 20180220
	int m_suggestDifficultyRequestId;
	
    // Mutexes.
    Mutex m_jobsMutex;  // Mutex for Stratum jobs and the related data.
    Mutex m_sharesMutex;  // Mutex for the queue of solutions.

    // Queue of jobs received from Pool.
    static const size_t c_maxJobs = 128;
    RingQueue<StratumJob, c_maxJobs> m_jobs;

    // Queue of shares to be send to Pool.
    static const size_t c_maxShares = 4096;
    RingQueue<StratumShare, c_maxShares> m_shares;

    // A counter to create internal job IDs.
    static const uint32_t c_firstJobIdCounter = 100;
    uint32_t m_jobIdCounter;

    // Extra-nonce 1 (unique per-connection) 
    uint32_t m_extraNonce1Size;
    uint8_t m_extraNonce1[c_extraNoncesMaxSize];

    // Extra-nonce 2 size in bytes.
    uint32_t m_extraNonce2Size;

    // Current difficulty from "mining.set_difficulty" message.
    volatile uint32_t m_difficulty;
    bool m_difficultyReceived;

    StratumReqMgr m_reqMgr;

    // Control counters.
    unsigned int m_connectionAttempts;
    unsigned int m_successiveRejects;

// Statistic counters since the last cut-off.
private:
    Mutex m_statMutex;  // Mutex for statistics data.
    
    // Statistic timers.
    PollTimer m_totalPoolTimer;  // Total elapsed time.
    PollTimer m_inServiceTimer;  // Accumulated time when the pool was IN_SERVICE state.
    
    unsigned int m_receivedJobs;
    unsigned int m_receivedJobsWithClean;
    
    unsigned int m_sentShares;
    unsigned int m_acceptedShares;
    unsigned int m_rejectedShares;
    uint64_t m_acceptedSolutions;
};

#endif  // STRATUM_POOL_H
