
#include "openedit/editEvent.h"
#include "opentimelineio/serializableObject.h"

namespace openedit { namespace OPENEDIT_VERSION {

EditEventPtr EditEvent::create(
        Composition* parent,
        Item* composable,
        std::string const& kind,
        int index,
        optional<TimeRange> const& source_range)
{
    return std::shared_ptr<EditEvent>(new EditEvent(
        parent,
        composable,
        kind,
        index,
        source_range
    ));
}


EditEvent::EditEvent(
        Composition* parent,
        Item* composable,
        std::string const& kind,
        int index,
        optional<TimeRange> const& source_range)
    : _parent(parent)
    , _item(composable)
    , _kind(kind)
    , _index(index)
    , _source_range(source_range)
{
    if (_item)
        _original_range = _item.value->source_range();
}


EditEvent::~EditEvent()
{
    // Clean up if required.
    if (_item) {
        Item* item = _item.take_value();
        if (_kind == EditEventKind::remove)
            item->possibly_delete();
    }
}


bool EditEvent::run(ErrorStatus *error_status) {
    return false; // TODO
}


bool EditEvent::revert() {
    return false; // TODO
}

} }