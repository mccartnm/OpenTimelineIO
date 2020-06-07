#include "opentimelineio/event/event.h"
#include "opentimelineio/event/registry.h"
#include "opentimelineio/event/eventType.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


Event::Event(EventType* event_type, std::string const& name, AnyDictionary const& metadata)
    : _event_type(event_type)
    , _has_run(false)
    , Parent(name, metadata)
{}

void Event::forward(ErrorStatus *error_status) {
    _event_type->forward(error_status);
    if (!error_status) {
        _has_run = true;
    }
}

void Event::reverse(ErrorStatus *error_status) {
    _event_type->reverse(error_status);
    if (!error_status) {
        _has_run = false;
    }
}

bool Event::read_from(Reader &reader) {
    std::string type_id;
    if (Parent::read_from(reader)) {
        reader.read("type_id", &type_id);

        _event_type = EventTypeRegistry::get().createType(type_id);
        if (!_event_type) {
            reader.error(ErrorStatus::UNKOWN_EVENT_TYPE);
            return false;
        } else {
            _event_type->read_from(reader);
        }
    }
    return true;
}

void Event::write_to(Writer &writer) const {
    Parent::write_to(writer);
    if (_event_type) {
        writer.write("type_id", _event_type->type_id());
        _event_type->write_to(writer);
    }
}

} }