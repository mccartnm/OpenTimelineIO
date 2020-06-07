#pragma once

#include "opentimelineio/event/event.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

/*
    EventType is:
    - Atomic
        - The actuall procedure of an Event, this maintains the
          same rule set.
    - Stateless
        - Unlike the Event, this has zero concept of state
        - Does not know when it's been run
*/
class EventType {
public:
    EventType(std::string const& type_id);

    virtual void forward(ErrorStatus *error_status) = 0; 
    virtual void reverse(ErrorStatus *error_status) = 0; 

    std::string const& type_id() const { return _type_id; }

protected:
    friend class Event;
    virtual bool read_from(SerializableObject::Reader&);
    virtual void write_to(SerializableObject::Writer&) const;

private:
    std::string _type_id;

};

} }