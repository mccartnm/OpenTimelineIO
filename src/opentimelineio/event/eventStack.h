#pragma once

#include "opentimelineio/event/eventType.h"
#include "opentimelineio/event/registry.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

class EventStack : public EventType {
public:
    EventStack(std::string const& type_id);

    void add_event(RetainedEvent event);
    void add_event(EventType* event_type,
                   std::string const& name = std::string(),
                   AnyDictionary const& metadata = AnyDictionary());

    virtual void forward(ErrorStatus *error_status) override;
    virtual void reverse(ErrorStatus *error_status) override;

protected:
    virtual bool read_from(SerializableObject::Reader&);
    virtual void write_to(SerializableObject::Writer&) const;

private:
    std::vector<RetainedEvent> _events;

};

} }
