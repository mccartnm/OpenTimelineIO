#include "opentimelineio/event/eventStack.h"
#include "opentimelineio/event/registry.h"

#include <iterator>
#include <memory>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

template <class ReverseIterator>
typename ReverseIterator::iterator_type make_forward(ReverseIterator rit)
{
    return --(rit.base());
}

EventStack::EventStack(std::vector<Event*> const& events,
                       std::string const& name,
                       AnyDictionary const& metadata)
    : Parent(name, metadata)
    , _events(events.begin(), events.end())
{
}

void EventStack::add_event(RetainedEvent event) {
    _events.push_back(event);
}

void EventStack::forward(ErrorStatus *error_status) {
    auto event_it = _events.begin();

    for (; event_it != _events.end(); event_it++) {
        (*event_it).value->run(error_status);

        if (!error_status && event_it != _events.begin()) {
            std::reverse_iterator<decltype(event_it)> revent_it(event_it - 1);
            std::unique_ptr<ErrorStatus> rev_error(new ErrorStatus());

            for (; revent_it != _events.rend(); revent_it++) {
                (*revent_it).value->revert(rev_error.get());
                if (!rev_error.get()) {
                    return;
                }
            }
            return;
        }
    }
}

void EventStack::reverse(ErrorStatus *error_status) {
    auto event_it = _events.rbegin();

    for (; event_it != _events.rend(); event_it++) {
        (*event_it).value->run(error_status);

        if (!error_status && event_it != _events.rbegin()) {
            // Reverse a reverse iterator...
            decltype(_events)::iterator fevent_it = make_forward(event_it + 1);
            std::unique_ptr<ErrorStatus> fev_error(new ErrorStatus());

            for (; fevent_it != _events.end(); fevent_it++) {
                (*fevent_it).value->revert(fev_error.get());
                if (!fev_error.get()) {
                    return;
                }
            }
            return;
        }
    }
}

bool EventStack::read_from(SerializableObject::Reader &reader) {
    // FIXME: Will probably have to introduce child <-> parent hookups
    return reader.read("events", &_events);
}

void EventStack::write_to(SerializableObject::Writer &writer) const {
    writer.write("events", _events);
}

// -- Register
REGISTER_EVENT("EventStack", EventStack);

} }
