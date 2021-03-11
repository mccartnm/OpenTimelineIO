#pragma once

#include "opentimelineio/edit/edit.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

class ModifyItemSourceRangeEdit : public ItemEdit {
public:
    struct Schema {
        static constexpr auto name = "ModifyItemSourceRangeEdit";
        static constexpr int version = 1;
    };

    using Parent = ItemEdit;

    ModifyItemSourceRangeEdit(Item* item = nullptr,
                              optional<TimeRange> const& new_range = nullopt,
                              std::string const& name = std::string(),
                              AnyDictionary const& metadata = AnyDictionary());

protected:
    virtual void forward(ErrorStatus *error_status) override;
    virtual void reverse(ErrorStatus *error_status) override;

    virtual bool read_from(Reader&) override;
    virtual void write_to(Writer&) const override;

private:
    optional<TimeRange> _new_range;
    optional<TimeRange> _original_range;
};

} }
