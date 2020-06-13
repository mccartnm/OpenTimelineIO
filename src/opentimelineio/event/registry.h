#pragma once

/*
    Plugin system for building event dynamically both at the C++
    and extended binding level (e.g. python)

    All event creation routes through here to allow languages to
    define their own event types without needing any additional
    cruft
*/
#include "opentimelineio/version.h"
#include "opentimelineio/typeRegistry.h"
#include "opentimelineio/event/event.h"

#include <memory>
#include <utility>
#include <functional>
#include <unordered_map>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

/*
    Structure for upgrading an event's schema
    {
        { 2, [](AnyDictionary *d) { ... } },
        { 3, [](AnyDictionary *d) { ... } },
    }
*/
using UpgradeCallback = std::pair<int, std::function<void(AnyDictionary*)>>;
using UpgradeCallbacks = std::vector<UpgradeCallback>;

namespace event_factory {
/*
    Namespace for event factory management. Anything dealing with
    registering event types to the otio registry goes here. We
    keep them apart for brevity. This system saves us from including
    each and every header for C++ schema to register
*/
class _Factory {
public:
    _Factory(const char *id, UpgradeCallbacks upgrades)
        : _id(id), _upgrades(upgrades) {}
    virtual ~_Factory() {}

    const char *id() const { return _id; }
    UpgradeCallbacks upgrades() const { return _upgrades; }
    virtual void register_event(TypeRegistry*) = 0;

private:
    const char *_id;
    UpgradeCallbacks _upgrades;
};

template<typename T>
class Factory : public _Factory {
public:
    Factory(const char *id, UpgradeCallbacks upgrades)
        : _Factory(id, upgrades) {}

    void register_event(TypeRegistry *registry) override {
        registry->register_type<T>();
        for (UpgradeCallback &cb: upgrades()) {
            registry->register_upgrade_function(id(), cb.first, cb.second);
        }
    }
};

class _EventRegister {
protected:
    void add_event_class(_Factory *factory);
};

template<typename T>
class EventRegsiter : public _EventRegister {

public:
    explicit EventRegsiter(const char *id, UpgradeCallbacks upgrades = {}) {
        static_assert(std::is_base_of<Event, T>::value,
                      "Class is not a subclass of Event");
        add_event_class(new Factory<T>(id, upgrades));
    }
};

} // namespace event_factory


class EventRegistry {
public:
    static EventRegistry &get();

protected:
    friend class event_factory::_EventRegister;
    void add_event_class(
        event_factory::_Factory *event_type);

    friend class TypeRegistry;
    void register_events(TypeRegistry *registry);

private:
    explicit EventRegistry() {}

    using EventFactory = std::shared_ptr<event_factory::_Factory>;
    std::unordered_map<std::string, EventFactory> _event_registers;
};


// For C++ defined events
#define REGISTER_EVENT(ID, CLS, ...)\
    static event_factory::EventRegsiter<CLS> s_event##CLSRegster(ID, __VA_ARGS__);

} }