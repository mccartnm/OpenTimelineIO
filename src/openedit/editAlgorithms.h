#pragma once

#include "opentime/rationalTime.h"

#include "openedit/version.h"
#include "openedit/errorStatus.h"
#include "openedit/editEvent.h"

namespace openedit { namespace OPENEDIT_VERSION {

using RetainedItem = SerializableObject::Retainer<Item>;
using RetainedComposition = SerializableObject::Retainer<Composition>;
using RetainedComposable = SerializableObject::Retainer<Composable>;

struct PlacementKind
{
    static auto constexpr overwrite = "Overwrite";  //< Overwrite existing items
    static auto constexpr insert = "Insert";        //< Shift everything
    // static auto constexpr fill = "InFill";          //< Fill in a gap based on the in point
    // static auto constexpr fill_out = "OutFill";     //< Fill in a gap based on the out point
    // static auto constexpr fail = "Fail";            //< Fail if non-gap contact made
};

/*
    Place a composable item onto a composition at a given RationalTime
*/
EditEvents place(
        Item* child,
        Composition* into,
        RationalTime const& at,
        ErrorStatus* error_status,
        std::string const& placement = PlacementKind::overwrite,
        RetainedItem fill_template = nullptr,
        bool preview = false);

} }
