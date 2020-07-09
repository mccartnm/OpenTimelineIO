#include "opentimelineio/edit/editAlgorithms.h"

// #include "opentimelineio/gap.h"
// #include "opentimelineio/item.h"
// #include "opentimelineio/track.h"

#include <memory>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


namespace {
/*
    An intersection utility with ranges that correspond based
    on the IntersectionType.
*/
struct Intersection
{
    enum IntersectionType {
        None = -1,
        Contains,           //< range contains another item
        Contained,          //< range is contained by an item
        OverlapBefore,      //< range overlaps item and (starts first or begins)
        OverlapAfter,       //< range overlaps item and (ends second or ends)
    };

    IntersectionType type;
    RetainedItem const& item;
    optional<TimeRange> source_range_before;
    optional<TimeRange> source_range_after;

    Intersection(
        IntersectionType t,
        RetainedItem const& i,
        optional<TimeRange> source_before = nullopt,
        optional<TimeRange> source_after = nullopt)
        : type(t)
        , item(i)
        , source_range_before(source_before)
        , source_range_after(source_after)
        {}
};
using Intersections = std::vector<Intersection>;

Intersections get_intersections(
        Track *track,
        TimeRange const& track_range,
        ErrorStatus* error_status,
        int count = -1)
{
    std::vector<RetainedComposable> const& children = track->children();
    Intersections intersections;
    bool contact = false;

    for (size_t idx = 0; idx < children.size(); idx++) {
        RetainedComposable const& composable = children[idx];

        // FIXME: Handle transitions
        RetainedItem child = dynamic_cast<Item *>(composable.value);
        if (!child)
            continue;

        auto child_range = track->range_of_child_at_index(int(idx), error_status);
        if (*error_status) {
            return {};
        }

        if (track_range.contains(child_range)) {
            contact = true;
            intersections.push_back({
                Intersection::Contains,
                child
            });

        } else if (child_range.contains(track_range)) {
            contact = true;
            Intersection::IntersectionType type = Intersection::Contained;

            // WARNING:
            // This is iffy at best. Needs proofing
            //

            TimeRange source_before = {
                child_range.start_time(),
                (track_range.start_time() - child_range.start_time())
            };
            TimeRange source_after = {
                track_range.end_time_exclusive(),
                child_range.end_time_exclusive() - child_range.end_time_exclusive()
            };

            if (child_range.begins(track_range.start_time())) {
                // child_range.finishes() means Contains which is handled above
                type = Intersection::OverlapBefore;
            }
            else if (track_range.finishes(child_range)) {
                type = Intersection::OverlapAfter;
            }

            intersections.push_back({
                type,
                child,
                source_before,
                source_after
            });

        } else if (track_range.overlaps(child_range)) {
            contact = true;
            
            Intersection::IntersectionType type;
            TimeRange source_before, source_after;

            if (track_range.start_time() < child_range.start_time()) {
                // The new range starts _before_ this child
                type = Intersection::OverlapBefore;
                source_after = TimeRange(
                    track_range.end_time_exclusive(),
                    (child_range.end_time_exclusive() - track_range.end_time_exclusive())
                );

            } else {
                // .contains() above takes care of other cases
                type = Intersection::OverlapAfter;
                source_before = TimeRange(
                    child_range.start_time(),
                    (track_range.start_time() - child_range.start_time())
                );
            }

            intersections.push_back({
                type,
                child,
                source_before,
                source_after
            });

        } else {
            // No intersections
            if (contact)
                break; // Passed all intersections
        }
    }

}

} // anon utility namespace

/* -------------------------------------------------------------------------- */

EventStack* overwrite(Item* item,
                      Track* track,
                      optional<RationalTime> const& track_time,
                      ErrorStatus *error_status,
                      Item* fill_template,
                      bool preview)
{
    auto item_range = item->source_range();
    if (!item_range) {
        *error_status = ErrorStatus::INVALID_TIME_RANGE;
        return nullptr;
    }

    TimeRange place_range = TimeRange(trace_time, (*item_range).duration());
    auto intersections = get_intersections(track, place_range, error_status);
    if (*error_status)
        return nullptr;

    std::unique_ptr<EventStack> stack(new EventStack({}, "overwrite"));

    // __TODO__ Fill me in

    if (!preview) {
        stack->run(error_status);
        if (*error_status) {
            return nullptr;
        }
    }
    return stack.release();
}

} }
