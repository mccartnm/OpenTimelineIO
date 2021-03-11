#include "opentimelineio/event/event.h"
#include "opentimelineio/event/registry.h"

#define UNUSED(x) (void)x

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


Event::Event(std::string const& name, AnyDictionary const& metadata)
    : _has_run(false)
    , Parent(name, metadata)
{}


void Event::run(ErrorStatus *error_status) {
    if (_has_run) {
        *error_status = ErrorStatus::INVALID_EXECUTION_ORDER;
        return;
    }
    if (!_validate(error_status)) {
        return;
    }
    forward(error_status);
    if (!(*error_status))
        _has_run = true;
}

void Event::revert(ErrorStatus *error_status) {
    if (!_has_run) {
        *error_status = ErrorStatus::INVALID_EXECUTION_ORDER;
        return;
    }

    if (!_validate(error_status)) {
        return;
    }

    reverse(error_status);
    if (!(*error_status))
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

// This is registered in typeRegistry.cpp so we don't need:
// REGISTER_EVENT("Event", Event);

} }