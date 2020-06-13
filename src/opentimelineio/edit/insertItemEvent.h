#pragma once

#include "opentimelineio/edit/edit.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

/* Perform an insertion based on index */
class InsertItemEdit : public ItemEdit {
public:
    struct Schema {
        static constexpr auto name = "InsertItemEdit";
        static constexpr int version = 1;
    };

    using Parent = ItemEdit;

    InsertItemEdit(int index = 0,
                   Track* track = nullptr,
                   Item* item = nullptr,
                   std::string const& name = std::string(),
                   AnyDictionary const& metadata = AnyDictionary());

protected:
    virtual void forward(ErrorStatus *error_status) override;
    virtual void reverse(ErrorStatus *error_status) override;

    virtual bool read_from(Reader&) override;
    virtual void write_to(Writer&) const override;

private:
    int _index;
};

} }
