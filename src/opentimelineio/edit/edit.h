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

    Track* track() { return _track.value; }
    Item* item() { return _item.value; }

protected:
    void set_track(Track* track);
    void set_item(Item* item);

    void set_requires_track(bool required);

    virtual bool read_from(Reader&);
    virtual void write_to(Writer&) const;

    bool _validate(ErrorStatus *error_status) override;

private:
    bool _require_track = true;

    RetainedTrack _track = nullptr;
    RetainedItem _item = nullptr;
};


} }
