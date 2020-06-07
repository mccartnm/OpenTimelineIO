#pragma once
/*
    Plugin system for building event dynamically both at the C++
    and extended binding level (e.g. python)

    All event creation routes through here to allow languages to
    define their own schema overloads without needing any additional
    cruft
*/

#include "opentimelineio/version.h"
#include "opentimelineio/event/event.h"

#include <memory>
#include <unordered_map>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

namespace event_factory {

class _Factory {
public:
    _Factory(const char *id) : _id(id) {}
    virtual ~_Factory() {}

    const char *id() const { return _id; }
    virtual RetainedEvent createEvent(
        std::string &name, AnyDictionary
        const& metadata = AnyDictionary()) = 0;

private:
    const char *_id;
};

template<typename T>
class Factory : public _Factory {
public:
    Factory(const char *id) : _Factory(id) {}

    RetainedEvent createEvent(
        std::string &name, AnyDictionary
        const& metadata = AnyDictionary()
    ) override {
        return RetainedEvent(new T(id(), name, metadata));
    }
};

class _EventRegisterAbstract {
protected:
    void registerEventType(_Factory *factory);
};

template<typename T>
class EventRegsiter : public _EventRegisterAbstract {

public:
    explicit EventRegsiter(const char *id)
        : _id(id)
    {
        static_assert(std::is_base_of<Event, T>::value,
                      "Class is not a subclass of Event");
        registerEventType(new Factory<T>(id))
    }
};

} // namespace event_factory


class EventTypeRegistry {
public:
    static EventTypeRegistry &get();

    bool has_event_type(std::string const& type) const;

    // ??
    // void deregisterEventType(std::string const& type);

protected:
    friend class event_factory::_EventRegisterAbstract;
    void registerEventType(
        event_factory::_Factory *event_type);

private:
    explicit EventTypeRegistry() {}

    using EventFactory = std::shared_ptr<event_factory::_Factory>;
    std::unordered_map<std::string, EventFactory> _event_factories;
};


// For C++ defined events
#define REGISTER_EVENT_TYPE(ID, CLS)\
    static EventInterface::EventRegsiter<CLS>(ID) s_event##IDRegster(ID);

} }