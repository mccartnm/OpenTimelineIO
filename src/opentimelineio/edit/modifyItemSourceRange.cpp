#include "opentimelineio/edit/modifyItemSourceRange.h"

#include "opentimelineio/event/registry.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

ModifyItemSourceRangeEdit::ModifyItemSourceRangeEdit(
    Item* item,
    optional<TimeRange> const& new_range,
    std::string const& name,
    AnyDictionary const& metadata)
    : Parent(nullptr, item, name, metadata)
    , _new_range(new_range)
{
    set_requires_track(false);
}

void ModifyItemSourceRangeEdit::forward(ErrorStatus *error_status)
{
    _original_range = item()->source_range();
    item()->set_source_range(_new_range);
}

void ModifyItemSourceRangeEdit::reverse(ErrorStatus *error_status)
{
    item()->set_source_range(_original_range);
}

bool ModifyItemSourceRangeEdit::read_from(Reader& reader/*, EventContext& context*/) {
    return Parent::read_from(reader);
}

void ModifyItemSourceRangeEdit::write_to(Writer& writer) const {
    Parent::write_to(writer);
    return;
}

REGISTER_EVENT("ModifyItemSourceRangeEdit", ModifyItemSourceRangeEdit);

} }