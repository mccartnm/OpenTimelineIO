#include "opentimelineio/event/event.h"
#include "opentimelineio/event/registry.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


Event::Event(std::string const& name, AnyDictionary const& metadata)
    : _has_run(false)
    , Parent(name, metadata)
{}


void Event::run(ErrorStatus *error_status) {
    forward(error_status);
    if (!error_status)
        _has_run = true;
}

void Event::revert(ErrorStatus *error_status) {
    reverse(error_status);
    if (!error_status)
        _has_run = false;
}

bool Event::read_from(Reader &reader) {
    return Parent::read_from(reader);
}

void Event::write_to(Writer &writer) const {
    Parent::write_to(writer);
}

} }