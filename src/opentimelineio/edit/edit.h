#pragma once

#include "opentimelineio/optional.h"
#include "opentimelineio/track.h"
#include "opentimelineio/item.h"
#include "opentimelineio/event/event.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

using RetainedItem = SerializableObject::Retainer<Item>;
using RetainedTrack = SerializableObject::Retainer<Track>;

/* Abstract Event for editing an Item */
class ItemEdit : public Event {
public:
    struct Schema {
        static constexpr auto name = "ItemEdit";
        static constexpr int version = 1;
    };

    using Parent = Event;

    ItemEdit(Track* track = nullptr,
             Item* item = nullptr,
             std::string const& name = std::string(),
             AnyDictionary const& metadata = AnyDictionary());
    virtual ~ItemEdit();

    RetainedTrack const& get_track() const { return _track; }
    RetainedItem const& get_item() const { return _item; }

protected:
    Track* track() { return _track.value; }
    Item* item() { return _item.value; }

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

private:
    RetainedTrack _track = nullptr;
    RetainedItem _item = nullptr;
};


} }
