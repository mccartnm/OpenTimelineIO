#include "opentimelineio/event/eventType.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

EventType::EventType(std::string const& type_id)
    : _type_id(type_id)
{}

bool EventType::read_from(SerializableObject::Reader&) {
    return true;
}

void EventType::write_to(SerializableObject::Writer&) const {}

} }
