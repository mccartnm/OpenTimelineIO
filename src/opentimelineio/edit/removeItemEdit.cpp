#include "opentimelineio/edit/removeItemEdit.h"

#include "opentimelineio/event/registry.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

RemoveItemEdit::RemoveItemEdit(
    Item* item,
    std::string const& name,
    AnyDictionary const& metadata)
    : Parent(nullptr, item, name, metadata)
    , _forward_index(-1)
{
}

void RemoveItemEdit::forward(ErrorStatus *error_status)
{
    // Q: Should this be found in the constructor?
    _forward_index = track()->index_of_child(item(), error_status);
    if (*error_status) {
        return;
    }
    track()->remove_child(_forward_index, error_status);
}

void RemoveItemEdit::reverse(ErrorStatus *error_status)
{
    auto use_item = item();
    if (!use_item) {
        *error_status = ErrorStatus::UNRESOLVED_OBJECT_REFERENCE;
        return;
    }
    track()->insert_child(_forward_index, use_item, error_status);
}

bool RemoveItemEdit::read_from(Reader& reader) {
    reader.read("fw_index", &_forward_index);
    return Parent::read_from(reader);
}

void RemoveItemEdit::write_to(Writer& writer) const {
    Parent::write_to(writer);
    writer.write("fw_index", _forward_index);
    return;
}

REGISTER_EVENT("RemoveItemEdit", RemoveItemEdit);

} }