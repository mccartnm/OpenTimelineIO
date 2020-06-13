#include "opentimelineio/event/registry.h"

#include <mutex>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

// Minor mutex for get operators
std::mutex g_mutex;

void event_factory::_EventRegister::add_event_class(
    event_factory::_Factory *factory)
{
    EventRegistry::get().add_event_class(factory);
}

EventRegistry &EventRegistry::get() {
    static EventRegistry *s_register = nullptr;

    if (!s_register) {
        std::lock_guard<std::mutex> gaurd(g_mutex);
        if (!s_register) {
            s_register = new EventRegistry();
        }
    }

    return *s_register;
}

void EventRegistry::add_event_class(event_factory::_Factory *event_type) {
    _event_registers.emplace(event_type->id(), EventFactory(event_type));
}

void EventRegistry::register_events(TypeRegistry *registry) {
    for (auto pair: _event_registers) {
        pair.second->register_event(registry);
    }
    // Remove registered for clean up?
}

} }
