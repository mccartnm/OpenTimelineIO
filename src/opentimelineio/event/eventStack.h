#pragma once

#include "opentimelineio/event/event.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

class EventStack : public Event {
public:
    struct Schema {
        static auto constexpr name = "EventStack";
        static int constexpr version = 1;
    };

    using Parent = Event;

    EventStack(std::vector<Event*> const& events = std::vector<Event*>(),
               std::string const& name = std::string(),
               AnyDictionary const& metadata = AnyDictionary());
    virtual ~EventStack();

    void add_event(RetainedEvent event);

protected:
    void forward(ErrorStatus *error_status) override;
    void reverse(ErrorStatus *error_status) override;

    bool read_from(SerializableObject::Reader&) override;
    void write_to(SerializableObject::Writer&) const override;

private:
    std::vector<RetainedEvent> _events;

};

} }
