#ifndef APPLICATION_IMPL_H
#define APPLICATION_IMPL_H
/*
 * Contains ApplicationImpl class declaration.
 */

#include "base/NonCopyable.h"
#include "base/StringBuffer.h"

#include "config/Config.h"
#include "config/LockedConfig.h"

#include "app/AppRegistry.h"
#include "app/AppComponent.h"

#include "events/EventManager.h"
#include "env/GpioPolling.h"
#include "pool/StratumPool.h"

#include "sys/Mutex.h"

#include <limits.h>

#define MINER_READY_TIME 300
// The actual implementor of the Application class logic.
//
class ApplicationImpl
    : private NonCopyable  // Prevent copy and assignment.
{
friend class Application;

// Construction/destruction.
private:
    ApplicationImpl();
    ~ApplicationImpl() throw();

// ConfigHandler implementation.
protected:
    // Processes notification of the application configuration change.
    virtual void onConfigChange(Config& config);

// Class interface.
private:
    static const Config& defaults() throw();
    const LockedConfig config();
    Config& configRW();
    void parseCommandLine(int argc, const char* argv[]);
    
    void configPostProcessing();
    void init();
    void run();
    void done() throw();
    void requestExit(int rc);

    // Component getters.
    inline EventManager& events() throw()  { return m_eventManager; }
    inline GpioPolling& gpioPolling() throw()  { return m_gpioPolling; }
    inline StratumPool& pool() throw()  { return m_stratumPool; }

// Implementation methods.
private:
    void start();
    void stop();
    
    void appMain();
    
    void ensureInitialized();
    
    void configureLogging(const Config& config);
    void updateConfig(const Config& config);
	int checkFanSpeedThreshold(uint8_t temp);
    
    // Represents a set of action types processing a change of configuration.
    struct ConfigActions
    {
        // Creates an empty set of actions.
        ConfigActions();
        
        // Returns true if at least one action is set.
        bool operator!() const throw();
        
        // Supported action types.
        // The values are assigned sequentially staring from 0.
        // They are used as indexes in m_actionFlags array.
        // 
        // Warning: When adding new Type, do NOT forget to update
        // typeToString() method accordingly!
        //
        enum Type
        {
            NONE = 0,                   // No config change detected.
            AUTO,                       // No specific action is needed.
            EXIT_APP,                   // The application needs to restart.
            UPDATE_LOG_CFG,             // Must update logging settings.
            UPDATE_POOL_CFG,            // Must update Pool settings.
            RESTART_POOL,               // Must restart Pool.
            
            // A number of actions.
            NUMBER_OF_ACTIONS
        };
        
        void set(Type value);
        bool isSet(Type value) const;
        
        static const char* typeToString(Type value) throw();
    
    private:
        // Flags enabling/disabling a particular action.
        bool m_actionFlags[NUMBER_OF_ACTIONS];
    };
    
    static const ConfigActions analyzeConfigChange(
        const Config& config, const Config& oldConfig);
    
    void processConfigActions(const ConfigActions& actions,
        Config& config, const Config& oldConfig);

// Member variables.
private:
    // A registry of the managed application components.
    AppRegistry m_appRegistry;
    
    // Whether the application has been initialized.
    bool m_initialized;
    
    // Whether the application is started.
    bool m_started;
    
    // Application exit flag and return code.
    volatile bool m_needExit;
    volatile int m_returnCode;
    
    // Configuration instance.
    Config m_config;
    
    // Mutex to protect application config data.
    Mutex m_configMutex;
    
    // Application components.
    EventManager m_eventManager;
    GpioPolling m_gpioPolling;
    StratumPool m_stratumPool;
};

#endif  // APPLICATION_IMPL_H
