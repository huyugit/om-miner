/*
 * Contains AppRegistry class definition.
 */

#include "AppRegistry.h"

#include "base/BaseUtil.h"

#include "app/AppComponent.h"


AppRegistry::AppRegistry()
    : m_count(0)
    , m_position(0)
    , m_reverse(false)
{
    for (size_t i = 0; i < util::arrayLength(m_appComponents); ++i)
        m_appComponents[i] = nullptr;
}

void AppRegistry::registerComponent(AppComponent* appComponent)
{
    assert(appComponent != nullptr);
    assert(m_count < util::arrayLength(m_appComponents));

    m_appComponents[m_count++] = appComponent;
}

bool AppRegistry::selectComponents(bool reverse /*= false*/)
{
    m_position = (reverse) ? m_count : 0;
    m_reverse = reverse;
    return (m_count > 0);
}

AppComponent* AppRegistry::nextComponent()
{
    if (m_reverse)
    {
        return (m_position > 0) ? m_appComponents[--m_position] : nullptr;
    }
    else
    {
        return (m_position < m_count) ? m_appComponents[m_position++] : nullptr;
    }
}
