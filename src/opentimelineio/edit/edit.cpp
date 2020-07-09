
#include "opentimelineio/edit/edit.h"
#include "opentimelineio/event/registry.h"
#include "opentimelineio/serializableObject.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


ItemEdit::ItemEdit(Track* track, Item* item, std::string const& name, AnyDictionary const& metadata)
    : _track(track)
    , _item(item)
    , Parent(name, metadata)
{
    if (!_track && _item) {
        track = dynamic_cast<Track*>(_item.value->parent());
        if (track) {
            _track = track;
        }
    }
}

ItemEdit::~ItemEdit() {
    // Possibly check out the track and item to see if we can safely
    // remove them... TODO?
}

void ItemEdit::set_track(Track* track)
{
    _track = track;
}

void ItemEdit::set_item(Item* item)
{
    _item = item;
}

void ItemEdit::set_requires_track(bool required)
{
    _require_track = required;
}

bool ItemEdit::read_from(Reader& reader/*, EventContext& context*/) {
    //
    // REQUIRES: ID (Or symbol) System for item context
    //
    // std::string track_id;
    // reader.read("track", &track_id);
    // _track = context.from_id<Track>(track_id);
    //
    // std::string item_id;
    // reader.read("item", &item_id);
    // _item = context.from_id<Item>(item_id);
    //
    return true;
}

void ItemEdit::write_to(Writer& writer) const {
    //
    // REQUIRES: ID System for item context
    //
    // writer.writer("track", _track->context_id());
    // writer.writer("item", _item->context_id());
    //
    return;
}

bool ItemEdit::_validate(ErrorStatus *error_status)
{
    if (!item()) {
        *error_status = ErrorStatus::UNRESOLVED_OBJECT_REFERENCE;
    }
    else if (!track()) {
        // TODO: Probably replace this with a smarter validation layer
        // In this event the item was added to a track over the span
        // of the stack. This shouldn't happen much in practice (what's)
        // the point of adding something you're removing?
        auto use_track = dynamic_cast<Track*>(item()->parent());
        if (use_track) {
            set_track(use_track);
        }
        else if (_require_track) {
            *error_status = {ErrorStatus::NOT_A_CHILD_OF, "item not parented to track"};
        }
    }
    return error_status->outcome == ErrorStatus::OK;
}

REGISTER_EVENT("ItemEdit", ItemEdit);

} }

