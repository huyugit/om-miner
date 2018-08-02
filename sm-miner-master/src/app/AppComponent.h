#ifndef APP_COMPONENT_H
#define APP_COMPONENT_H
/*
 * Contains AppComponent class declaration.
 */

#include "base/NonCopyable.h"

#include "except/ApplicationException.h"

#include "app/AppRegistry.h"

#include <assert.h>


// Forward declarations.
class Config;

// Base class for application components.
// 
class AppComponent
    : private NonCopyable  // Prevent copy and assignment.
{
// Construction/destruction.
// Instantiation is restricted to the derived classes.
protected:
    // Default constructor.
    AppComponent()
        : m_initialized(false)
        , m_started(false)
    {
    }
    
    // Constructor registering this application component in
    // in the supplied application registry (appRegistry argument).
    explicit AppComponent(AppRegistry& appRegistry)
        : m_initialized(false)
        , m_started(false)
    {
        appRegistry.registerComponent(this);
    }
    
    // Destructor.
    // Inherited classes MUST define their own destructor and call "done()"
    // method from it to ensure the component is uninitialized properly.
    virtual ~AppComponent() throw()
    {
        assert(!m_initialized);
    }

// Public interface.
public:
    // Initializes the component before running.
    // Calls doInit() to do the job.
    void init(const Config& config)
    {
        if (isInitialized())
            throw ApplicationException("Application component has been already initialized.");
        
        doInit(config);
        m_initialized = true;
    }

    // Activates the application component.
    // Calls doStart() to do the job.
    void start()
    {
        ensureInitialized();
        
        if (isStarted())
            throw ApplicationException("Application component is already started.");
        
        doStart();
        m_started = true;
    }

    // Deactivates the application component.
    // Calls doStop() to do the job.
    void stop() throw()
    {
        if (isStarted())
        {
            doStop();
            m_started = false;
        }
    }

    // Stops and then starts the application component.
    void restart()
    {
        stop();
        start();
    }

    // Uninitializes the application component.
    // Calls doDone() to do the job.
    void done() throw()
    {
        if (isInitialized())
        {
            stop();
            doDone();
            m_initialized = false;
        }
    }
    
    // Returns true if the application component has been initialized.
    bool isInitialized() const throw()
    {
        return m_initialized;
    }
    
    // Returns true if the application component is started.
    bool isStarted() const throw()
    {
        return m_started;
    }

// Protected methods that may be overridden in the derived classes.
protected:
    // Called to initialize the component before running.
    virtual void doInit(const Config& /*config*/)  {}
    
    // Called to activate the application component.
    virtual void doStart()  {}
    
    // Called to deactivate the application component.
    virtual void doStop() throw()  {}
    
    // Called to deinitialize the application component.
    // This method is guaranteed to be called only once even in case
    // of multiple calls to "done()".
    virtual void doDone() throw()  {}

// Implementation methods.
private:
    void ensureInitialized()
    {
        if (!isInitialized())
            throw ApplicationException("Application component is not initialized.");
    }

// Member variables.
private:
    // Whether the application component has been initialized.
    bool m_initialized;

    // Whether the application component is started.
    bool m_started;
};

#endif  // APP_COMPONENT_H
