#ifndef APP_REGISTRY_H
#define APP_REGISTRY_H
/*
 * Contains AppRegistry class declaration.
 */

#include <stdlib.h>


// Forward declarations.
class AppComponent;

// Class maintaining a list of the application components.
// 
class AppRegistry
{
// Construction/destruction.
public:
    // Default constructor.
    AppRegistry();

// Public interface.
public:
    // Registers the given component in the application registry.
    void registerComponent(AppComponent* appComponent);
    
// Iteration methods.
public:
    // Select the registered application component for iteration.
    // Returns true if there are available items to iterate.
    // If the "reverse" argument is true, the iteration is
    // performed in the reverse order of the registration.
    bool selectComponents(bool reverse = false);

    // Returns application component at the current iterator position
    // and moves the iterator to the next item. If there is no more
    // available component, returns nullptr.
    AppComponent* nextComponent();

// Member variables.
private:
    // A number of the registered application components.
    size_t m_count;
    
    // An array of pointers to the registered application component.
    static const size_t c_maxAppComponents = 16;
    AppComponent* m_appComponents[c_maxAppComponents];
    
    // Iteration state.
    size_t m_position;  // Iteration position in m_appComponents array.
    bool m_reverse;     // Where the iteration is performed in the reverse order.
};

#endif  // APP_REGISTRY_H
