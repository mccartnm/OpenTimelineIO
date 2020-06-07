#pragma once

#include "opentimelineio/version.h"
#include "opentimelineio/serializableObjectWithMetadata.h"


namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

/*
    Event Requirements:
    - Atomic
        - No outside dependencies (other than member lifetimes)
        - It should pass or fail completely
        - If failure occurs, rollback is mandatory
        - If all items have this, EventStack can also be an event
          which is the sum of all it's children events.
        - ***Note: Does _not_ require statelessness
    - Self-sufficient
        - Because of the rules above, an event should be consistently
          runnable both forward and reverse
        - **Not** required to handle the case of:

            event.forward()
            event.forward()

            or

            event.reverse()
            event.reverse()
*/
class Event : public SerializableObjectWithMetadata {
public:
    struct Schema {
        static auto constexpr name = "Event";
        static int constexpr version = 1;
    };

    using Parent = SerializableObjectWithMetadata;

    Event(std::string const& event_type,
          std::string const& name,
          AnyDictionary const& metadata = AnyDictionary());

    virtual void forward(ErrorStatus *error_status) = 0;
    virtual void reverse(ErrorStatus *error_status) = 0;

protected:
    virtual ~Event() {}

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

private:
    std::string _event_type;
};

using RetainedEvent = SerializableObject::Retainer<Event>;


} }