#include "opentimelineio/event/registry.h"

#include <mutex>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

// Minor mutex for get operators
std::mutex g_mutex;

void event_type_factory::_EventTypeRegister::registerEventType(
    event_type_factory::_Factory *factory)
{
    EventTypeRegistry::get().registerEventType(factory);
}

EventTypeRegistry &EventTypeRegistry::get() {
    static EventTypeRegistry *s_register = nullptr;

    if (!s_register) {
        std::lock_guard<std::mutex> gaurd(g_mutex);
        if (!s_register) {
            s_register = new EventTypeRegistry();
        }
    }

    return *s_register;
}


bool EventTypeRegistry::has_event_type(std::string const& type_id) const {
    auto it = _event_factories.find(type_id);
    return it != _event_factories.end();
}


void EventTypeRegistry::registerEventType(event_type_factory::_Factory *event_type) {
    _event_factories.emplace(event_type->id(), EventFactory(event_type));
}

EventType* EventTypeRegistry::createType(std::string const & type_id)
{
    auto it = _event_factories.find(type_id);
    if (it == _event_factories.end())
        return nullptr;
    return (*it).second->createType();
}

} }
