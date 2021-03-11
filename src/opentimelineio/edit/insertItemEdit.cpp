#include "opentimelineio/edit/insertItemEdit.h"

#include "opentimelineio/vectorIndexing.h"
#include "opentimelineio/event/registry.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

InsertItemEdit::InsertItemEdit(
    int index,
    Track* track,
    Item* item,
    std::string const& name,
    AnyDictionary const& metadata)
    : Parent(track, item, name, metadata)
{
    if (this->track()) {
        _index = adjusted_vector_index(index, this->track()->children());
    }
}

void InsertItemEdit::forward(ErrorStatus *error_status)
{
    auto use_track = track();
    if (_index > use_track->children().size()) {
        // Q: Should this just clamp?
        *error_status = ErrorStatus::ILLEGAL_INDEX;
        return;
    }
    use_track->insert_child(_index, item(), error_status);
}

void InsertItemEdit::reverse(ErrorStatus *error_status)
{
    track()->remove_child(_index, error_status);
}

bool InsertItemEdit::read_from(Reader& reader/*, EventContext& context*/) {
    reader.read("index", &_index);
    return Parent::read_from(reader);
}

void InsertItemEdit::write_to(Writer& writer) const {
    Parent::write_to(writer);
    writer.write("index", _index);
    return;
}

REGISTER_EVENT("InsertItemEdit", InsertItemEdit);

} }