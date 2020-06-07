#pragma once

#include "opentimelineio/optional.h"
#include "opentimelineio/track.h"
#include "opentimelineio/item.h"
#include "opentimelineio/event/eventType.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

using RetainedItem = SerializableObject::Retainer<Item>;
using RetainedTrack = SerializableObject::Retainer<Track>;

/*
    Abstract EventType for editing
*/
class Edit : public EventType {
public:
    RetainedTrack const& get_track() const { return _track; }
    RetainedItem const& get_item() const { return _item; }

protected:
    RetainedTrack& track() { return _track; }
    RetainedItem& item() { return _item; }

    virtual bool read_from(SerializableObject::Reader&);
    virtual void write_to(SerializableObject::Writer&) const;

private:
    RetainedTrack _track = nullptr;
    RetainedItem _item = nullptr;
};


} }