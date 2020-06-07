
#include "opentimelineio/edit/edit.h"
#include "opentimelineio/serializableObject.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

bool Edit::read_from(SerializableObject::Reader& reader/*, EventContext& context*/) {
    //
    // REQUIRES: ID System for item context
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


void Edit::write_to(SerializableObject::Writer& writer) const {
    //
    // REQUIRES: ID System for item context
    //
    // writer.writer("track", _track->context_id());
    // writer.writer("item", _item->context_id());
    //
    return;
}

} }