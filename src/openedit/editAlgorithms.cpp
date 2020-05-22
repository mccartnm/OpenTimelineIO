#include "openedit/editAlgorithms.h"

#include "opentimelineio/gap.h"
#include "opentimelineio/item.h"

namespace openedit { namespace OPENEDIT_VERSION {

/* Implementation Utilities */
namespace {

struct Intersection
{
    enum IntersectionType {
        None = -1,
        Eclipse,
        SingleEdgeLeft,
        SingleEdgeRight,
        SplitTake,
    };

    IntersectionType type;
    RetainedItem const& item;
    optional<TimeRange> source_range_left = nullopt;
    optional<TimeRange> source_range_right = nullopt;

    TimeRange invalid_time() const {
        return TimeRange(RationalTime(0, -1), RationalTime(0, -1));
    }

    bool left_valid() const {
        return source_range_left.value_or(invalid_time()).start_time().is_invalid_time();
    }
};
using Intersections = std::vector<Intersection>;


Intersections get_intersections(
        Composition *track,
        TimeRange time_range,
        ErrorStatus* error_status)
{
    std::vector<RetainedComposable> const& children = track->children();
    Intersections intersections;

    bool contact = false;
    bool end_conact = false;

    for (size_t idx = 0; idx < children.size(); idx++) {

        RetainedComposable const& composable = children[idx];

        // FIXME: Handle transitions
        RetainedItem child = dynamic_cast<Item *>(composable.value);
        if (!child)
            continue;

        auto otio_error = ErrorStatus::otio_error_status();
        auto child_range = track->range_of_child_at_index(int(idx), otio_error.get());
        if (*(otio_error.get())) {
            *error_status = ErrorStatus(otio_error);
            return;
        }

        if (time_range.contains(child_range)) {
            contact = true;
            intersections.push_back({
                Intersection::Eclipse,
                child
            });
        }
        else if (child_range.contains(time_range)) {
            contact = true;
            Intersection::IntersectionType type = Intersection::SplitTake;
            if (time_range.start_time() == child_range.start_time()) {
                if (time_range.end_time_exclusive() == child_range.end_time_exclusive())
                    type = Intersection::Eclipse;
                else
                    type = Intersection::SingleEdgeRight;
            }
            else {
                type = Intersection::SingleEdgeLeft;
            }

            intersections.push_back({
                type,
                child,
                TimeRange{
                    child_range.start_time(),
                    (time_range.start_time() - child_range.start_time())
                },
                TimeRange{
                    time_range.end_time_exclusive(),
                    (child_range.end_time_exclusive() - time_range.end_time_exclusive())
                }
            });
        }
        else if (time_range.overlaps(child_range)) {
            contact = true;

            Intersection::IntersectionType type;
            TimeRange range_left, range_right;

            if (time_range.end_time_exclusive() > child_range.end_time_exclusive()) {
                type = Intersection::SingleEdgeLeft;
                range_left = TimeRange(
                    child_range.start_time(),
                    (time_range.start_time() - child_range.start_time())
                );
            }
            else {
                type = Intersection::SingleEdgeRight;
                range_right = TimeRange(
                    time_range.end_time_exclusive(),
                    (child_range.end_time_exclusive() - time_range.end_time_exclusive())
                );
            }
            intersections.push_back({
                type,
                child,
                range_left,
                range_right
            });
        }
        else {
            // No intersections
            if (contact)
                break; // Passed all intersections
        }
    }
    return intersections;
}

} // impl namespace


/* -------------------------------------------------------------------------- */


EditEvents place(
    Item* child,
    Composition* into,
    RationalTime const& at,
    ErrorStatus* error_status,
    std::string const& placement,
    RetainedItem fill_template,
    bool preview)
{
    auto otio_error = ErrorStatus::otio_error_status();

    //
    // First - We look for the items that we'll be intersecting
    // - To do this, we'll find the first item that we collide with
    //   and keep checking items until we no longer overlap
    //
    auto source_range = child->source_range();
    if (!source_range)
    {
        *error_status = ErrorStatus::NO_SOURCE_RANGE_ON_CHILD;
        return {};
    }

    auto child_range = source_range.value;
    auto intersections = get_intersections(into, child_range, error_status);
    if (*error_status)
        return {};

    EditEvents events;

    // FIXME: This seems less-than-ideal for preview=true...
    auto get_fill = [fill_template, error_status, otio_error]() {
        RetainedItem compose;
        if (!fill_template)
            compose = new Gap();
        else {
            auto cloned = fill_template.value->clone(otio_error.get());
            if (*(otio_error.get())) {
                *error_status = ErrorStatus(otio_error);
            }
            else {
                compose = static_cast<Item*>(cloned);
            }
        }
        return compose;
    };

    if (intersections.size() == 0) {
        //
        // We have nothing here, this means we're passed the
        // edge of the tracks trimmed range. Build a gap to fill the
        // space and call it a day
        //
        TimeRange range = into->available_range(otio_error.get());
        if (*(otio_error.get())) {
            *error_status = ErrorStatus(otio_error);
            return {};
        }

        RationalTime start = range.end_time_exclusive();
        RetainedItem retainer = get_fill();
        if (*error_status)
            return {};

        Item* fill = retainer.value;
        fill->set_source_range(range);

        events.push_back(EditEvent::create(
            into,
            fill,
            EditEventKind::append
        ));
        events.push_back(EditEvent::create(
            into,
            child,
            EditEventKind::append
        ));
    }
    else {

        //
        // -- This combination is the state driver for an action when we
        // have to modify the children of our composition.
        //
        // For each of the intersections, based on the type of placement
        // requested, we handle accordingly.
        //
        TimeRange default_range = TimeRange(RationalTime{ 0, 1 }, RationalTime{ 1, 1 }); // Kill this with fire

        if (placement == PlacementKind::overwrite) {
            //
            // -- First up, the overwrite command. This just blindly destroys
            // anything it overlaps. This can potentially mean splitting an
            // item in two or wiping out a child all together.
            //
            for (Intersection &intersect: intersections) {
                switch (intersect.type)
                {
                case Intersection::Eclipse:
                {
                    events.push_back({
                        into,
                        intersect.item,
                        EditEventKind::remove
                    });
                    break;
                }
                case Intersection::SingleEdgeLeft:
                case Intersection::SingleEdgeRight:
                {
                    TimeRange use_range = intersect.source_range_right.value_or(default_range);
                    if (intersect.type == Intersection::SingleEdgeLeft)
                        use_range = intersect.source_range_left.value_or(default_range);

                    events.push_back(EditEvent::create(
                        into,
                        intersect.item,
                        EditEventKind::modify,
                        -1,
                        use_range
                    ));
                    break;
                }
                case Intersection::SplitTake:
                {
                    auto split = intersect.item.value->clone(otio_error.get());
                    if (*(otio_error.get())) {
                        *error_status = ErrorStatus(otio_error);
                        return {};
                    }
                    Item* cloned_item = static_cast<Item*>(split);

                    int index = into->_index_of_child(intersect.item, otio_error.get());
                    if (*(otio_error.get())) {
                        *error_status = ErrorStatus(otio_error);
                        return {};
                    }

                    events.push_back(EditEvent::create(
                        into,
                        intersect.item,
                        EditEventKind::modify,
                        -1,
                        intersect.source_range_left
                    ));
                    events.push_back(EditEvent::create(
                        into,
                        child,
                        EditEventKind::insert,
                        index + 1
                    ));
                    events.push_back(EditEvent::create(
                        into,
                        cloned_item,
                        EditEventKind::insert,
                        index + 2,
                        intersect.source_range_right
                    ));
                    break;
                }
                case Intersection::None:
                default:
                    break;
                }
            }
        }
        else if (placement == PlacementKind::insert) {
            //
            // -- Next is the predicated insertion. This will insert the
            // item and shift _anything_ that it contacts. This can also
            // result in a split item.
            //
            Intersection &intersect = intersections[0]; // Empty handled above
            TimeRange composable_range = intersect.item.value->source_range().value_or(default_range);

            int index = into->_index_of_child(intersect.item, otio_error.get());
            if (*(otio_error.get())) {
                *error_status = ErrorStatus(otio_error);
                return {};
            }

            if (intersect.type == Intersection::Eclipse ||
                intersect.type == Intersection::SingleEdgeRight ||
                child_range.start_time() <= composable_range.start_time())
            {
                events.push_back(EditEvent::create(
                    into,
                    child,
                    EditEventKind::insert,
                    index
                ));
            }
            else { // SingleEdgeLeft || SplitTake
                events.push_back(EditEvent::create(
                    into,
                    child,
                    EditEventKind::insert,
                    index + 1
                ));

                RationalTime child_start_time = child->source_range().value_or(default_range).start_time();
                if (child_start_time < composable_range.end_time_exclusive())
                {
                    auto split = intersect.item.value->clone(otio_error.get());
                    if (*(otio_error.get())) {
                        *error_status = ErrorStatus(otio_error);
                        return {};
                    }

                    // FIXME: Need a better way to assert values...
                    TimeRange split_source_range = TimeRange(
                        intersect.source_range_left.value.end_time_exclusive(),
                        composable_range.duration() - intersect.source_range_left.value.duration()
                    );

                    events.push_back(EditEvent::create(
                        into,
                        child,
                        EditEventKind::modify,
                        -1,
                        intersect.source_range_left
                    ));
                    events.push_back(EditEvent::create(
                        into,
                        child,
                        EditEventKind::insert,
                        index + 2
                    ));
                }
            }
        }
    }

    if (events.size() && !preview)
    {
        decltype(events) completed;

        for (EditEventPtr event : events) {
            event->run(error_status);
            if (*error_status) {
                for (auto it = completed.rbegin(); it != completed.rend(); ++it)
                    (*it)->revert();
                return {};
            }
            completed.push_back(event);
        }
    }

    return events;

}


}}
