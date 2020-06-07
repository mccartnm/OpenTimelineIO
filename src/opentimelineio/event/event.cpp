#include "event.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


Event::Event(std::string const& event_type, std::string const& name, AnyDictionary const& metadata)
    : _event_type(event_type)
    , Parent(name, metadata)
{}


bool Event::read_from(Reader &reader) {
    return SerializableObjectWithMetadata::read_from(reader);
}


void Event::write_to(Writer &writer) const {
    return SerializableObjectWithMetadata::write_to(writer);
}

} }