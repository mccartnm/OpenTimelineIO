#include "registry.h"

#include <mutex>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

// Minor mutex for get operators
std::mutex g_mutex;

void event_factory::_EventRegisterAbstract::registerEventType(event_factory::_Factory *factory) {
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


bool EventTypeRegistry::has_event_type(std::string const& type) const {
    auto it = _event_factories.find(type);
    return it != _event_factories.end();
}


void EventTypeRegistry::registerEventType(event_factory::_Factory *event_type) {
    _event_factories.emplace(event_type->id(), EventFactory(event_type));
}



} }
