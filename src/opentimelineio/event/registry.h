#pragma once
/*
    Plugin system for building event dynamically both at the C++
    and extended binding level (e.g. python)

    All event creation routes through here to allow languages to
    define their own schema overloads without needing any additional
    cruft
*/

#include "opentimelineio/version.h"
#include "opentimelineio/event/eventType.h"

#include <memory>
#include <unordered_map>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

namespace event_type_factory {

class _Factory {
public:
    _Factory(const char *id) : _id(id) {}
    virtual ~_Factory() {}

    const char *id() const { return _id; }
    virtual EventType* createType() = 0;

private:
    const char *_id;
};

template<typename T>
class Factory : public _Factory {
public:
    Factory(const char *id) : _Factory(id) {}

    EventType* createType() override {
        return new T(id());
    }
};

class _EventTypeRegister {
protected:
    void registerEventType(_Factory *factory);
};

template<typename T>
class EventTypeRegsiter : public _EventTypeRegister {

public:
    explicit EventTypeRegsiter(const char *id) {
        static_assert(std::is_base_of<EventType, T>::value,
                      "Class is not a subclass of EventType");
        registerEventType(new Factory<T>(id));
    }
};

} // namespace event_type_factory


class EventTypeRegistry {
public:
    static EventTypeRegistry &get();
    bool has_event_type(std::string const& type_id) const;

    EventType* createType(std::string const& type_id);

protected:
    friend class event_type_factory::_EventTypeRegister;
    void registerEventType(
        event_type_factory::_Factory *event_type);

private:
    explicit EventTypeRegistry() {}

    using EventFactory = std::shared_ptr<event_type_factory::_Factory>;
    std::unordered_map<std::string, EventFactory> _event_factories;
};


// For C++ defined events
#define REGISTER_EVENT_TYPE(ID, CLS)\
    static event_type_factory::EventTypeRegsiter<CLS> s_event##CLSRegster(ID);

} }