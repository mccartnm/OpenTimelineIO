#pragma once

#include "opentime/rationalTime.h"
#include "opentimelineio/item.h"
#include "opentimelineio/track.h"
#include "opentimelineio/event/eventStack.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

// using RetainedItem = SerializableObject::Retainer<Item>;
// using RetainedComposition = SerializableObject::Retainer<Composition>;
// using RetainedComposable = SerializableObject::Retainer<Composable>;
// using RetainedEvent = SerializableObject::Retainer<Event>;

// struct PlacementKind
// {
//     static auto constexpr overwrite = "Overwrite";  //< Overwrite existing items
//     static auto constexpr insert = "Insert";        //< Shift everything
//     // static auto constexpr fill = "InFill";          //< Fill in a gap based on the in point
//     // static auto constexpr fill_out = "OutFill";     //< Fill in a gap based on the out point
//     // static auto constexpr fail = "Fail";            //< Fail if non-gap contact made
// };

// EditEvents place(
//         Item* child,
//         Track* track,
//         RationalTime const& at,
//         ErrorStatus* error_status,
//         std::string const& placement = PlacementKind::overwrite,
//         RetainedItem fill_template = nullptr,
//         bool preview = false);


/* Place a composable item onto a composition at a given RationalTime */
EventStack* overwrite(Item* item,
                      Track* track,
                      optional<RationalTime> const& track_time,
                      ErrorStatus *error_status,
                      Item* fill_template = nullptr,
                      bool preview = false);


} }
