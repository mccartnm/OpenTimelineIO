#include "opentimelineio/event/event.h"
#include "opentimelineio/event/registry.h"

#define UNUSED(x) (void)x

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

void Event::forward(ErrorStatus* error_status)
{
    UNUSED(error_status);
}

void Event::reverse(ErrorStatus* error_status)
{
    UNUSED(error_status);
}

bool Event::read_from(Reader &reader) {
    return Parent::read_from(reader);
}

void Event::write_to(Writer &writer) const {
    Parent::write_to(writer);
}

// DO NOT REGISTER - THIS IS THE BASE
// REGISTER_EVENT("Event", Event);

} }