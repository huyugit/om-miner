#ifndef APPLICATION_H
#define APPLICATION_H
/*
 * Contains Application class declaration.
 */

#include "base/NonCopyable.h"

#include "config/LockedConfig.h"

#include "app/AppComponent.h"

#include <stdlib.h>


// Shortcut for event reporting call.
#define REPORT_EVENT \
    Application::events().reportEvent


// Forward declarations.
class ApplicationImpl;
class Config;
class EventManager;
class GpioPolling;
class StratumPool;

// Singleton class managing the application components.
// The actual implementation is delegated to the ApplicationImpl class.
//
class Application
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction (private).
private:
    Application();
    ~Application() throw();

// Public interface.
public:
    // Returns configuration defaults.
    static const Config& defaults() throw();
    
    // Returns a proxy to the application config.
    // The proxy object locks the application config on construction
    // and unlocks on destruction (see LockedConfig for details).
    //
    // If you are going to access multiple config fields and to avoid
    // excessive locking/unlocking, store the LockedConfig instance
    // returned by config() in a local variable and then use that
    // variable to access individual fields. When the LockedConfig
    // variable goes out of scope, the referenced config is automatically
    // unlocked. Alternatively, we may to unlock it manually in advance
    // by calling LockedConfig unlock() method.
    //
    // E.g.:
    //     LockedConfig config = Application::config();
    //     param1_type param1_value = config->param1_name;
    //     param2_type param2_value = config->param2_name;
    //     ...
    //     config.unlock();
    //
    static const LockedConfig config();
    static Config& configRW();

    // Parses command line and updates configuration accordingly.
    static void parseCommandLine(int argc, const char* argv[]);
    
    // Initializes the application.
    static void configPostProcessing();
    static void init();
    
    // Runs the application.
    static void run();
    
    // Set a special flag requesting the application exit.
    // This flag is checked in the application main loop.
    // When the exit request is detected, the main loop
    // stops all the application components and exits.
    static void requestExit(int rc = EXIT_SUCCESS);
    
    // Uninitializes the application.
    static void done() throw();
    
    // Component getters.
    static EventManager& events() throw();
    static GpioPolling &gpioPolling() throw();
    static StratumPool& pool() throw();

// Implementation methods.
private:
    // Returns the Application singleton instance.
    static Application& instance() throw();

// Member variables.
private:
    // Implementation pointer.
    ApplicationImpl* m_implPtr;
};

#endif  // APPLICATION_H
