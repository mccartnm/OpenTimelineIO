#pragma once

#include "opentimelineio/optional.h"
#include "opentimelineio/track.h"
#include "opentimelineio/item.h"
#include "opentimelineio/event/event.h"

namespace openedit { namespace OPENEDIT_VERSION {

class Edit : public Event
{
public:

    Retainer<Track> const& get_track() const { return _track; }
    Retainer<Item> const& get_item() const { return _item; }

    // Execution Utils
    bool forward(ErrorStatus *error_status);
    bool reverse(ErrorStatus *error_status);

protected:
    Retainer<Track>& get_track() const { return _track; }
    Retainer<Item>& get_item() const { return _item; }

    virtual ~EditEvent();

    // EditEvent(
    //     Track* parent,
    //     Item* composable,
    //     std::string const& kind,
    //     int index,
    //     optional<TimeRange> const& source_range);

private:

    Retainer<Track> _track = nullptr;
    Retainer<Item> _item = nullptr;

    // int _index = -1;
    // optional<TimeRange> _source_range;
    // optional<TimeRange> _original_range;
};


} }