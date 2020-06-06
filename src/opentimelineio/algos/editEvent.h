#pragma once

#include "openedit/errorStatus.h"
#include "openedit/version.h"

#include "opentimelineio/optional.h"
#include "opentimelineio/serializableObject.h"
#include "opentimelineio/composition.h"
#include "opentimelineio/composable.h"
#include "opentimelineio/track.h"

namespace openedit { namespace OPENEDIT_VERSION {


struct EditEventKind {
    static auto constexpr none = "none";
    static auto constexpr insert = "insert";
    static auto constexpr append = "append";
    static auto constexpr remove = "remove";
    static auto constexpr modify = "modify";
};


class EditEvent;
using EditEventPtr = std::shared_ptr<EditEvent>;


/*
 * Something that occurs during an edit. These are externally
 * immutable to support reverting events.
*/
class EditEvent
{
public:
    static EditEventPtr create(
            Track* parent,
            Item* composable,
            std::string const& kind,
            int index = -1,
            optional<TimeRange> const& source_range = nullopt);

    ~EditEvent();

    SerializableObject::Retainer<Track> const& get_parent() const {
        return _parent;
    }

    SerializableObject::Retainer<Item> const& get_composable() const {
        return _item;
    }

    int get_index() const {
        return _index;
    }

    std::string const& get_kind() const {
        return _kind;
    }

    // Execution Utils
    bool run(ErrorStatus *error_status);
    bool revert();

protected:
    EditEvent(
        Track* parent,
        Item* composable,
        std::string const& kind,
        int index,
        optional<TimeRange> const& source_range);

private:

    SerializableObject::Retainer<Track> _parent = nullptr;
    SerializableObject::Retainer<Item> _item = nullptr;
    std::string _kind = EditEventKind::none;

    int _index = -1;
    optional<TimeRange> _source_range;
    optional<TimeRange> _original_range;
};

using EditEvents = std::vector<EditEventPtr>;


} }