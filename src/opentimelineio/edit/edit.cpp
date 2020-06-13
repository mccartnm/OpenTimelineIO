
#include "opentimelineio/edit/edit.h"
#include "opentimelineio/serializableObject.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


ItemEdit::ItemEdit(Track* track, Item* item, std::string const& name, AnyDictionary const& metadata)
    : _track(track)
    , _item(item)
    , Parent(name, metadata)
{    
}

ItemEdit::~ItemEdit() {
    // Possibly check out the track and item to see if we can safely
    // remove them... TODO?
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

} }