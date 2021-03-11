#include "opentimelineio/edit/editAlgorithms.h"

#include "opentimelineio/gap.h"
#include "opentimelineio/item.h"
#include "opentimelineio/track.h"

#include "opentimelineio/edit/insertItemEdit.h"
#include "opentimelineio/edit/modifyItemSourceRange.h"
#include "opentimelineio/edit/removeItemEdit.h"

#include <memory>
#include <vector>
#include <iostream>

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {


namespace edit_util {

/*
    Utility for inserting an item after all currently available
    track data.

    Given the scenario:

                        [  B  ]
                        ↓
    +--------------------------+
    |[   A   ]|
    +--------------------------+
             ⬆ End of current track

    We want to insert B by create a GAP (or provided fill_template)
    item to create the result:

    +--------------------------+
    |[   A   ][  GAP   ][  B  ]|
    +--------------------------+
*/
bool push_fill(EventStack *stack,
               Track* track,
               Item* item,
               Item* fill_template,
               const TimeRange &place_range,
               ErrorStatus *error_status)
{
    Gap::Retainer<Gap> g = nullptr;
    if (!fill_template) {
        g = new Gap();
        fill_template = g;
    }

    Item *fill = dynamic_cast<Item*>(fill_template->clone(error_status));
    if (*error_status || !fill)
        return false;

    RationalTime end_of_current = track->available_range(error_status).end_time_exclusive();
    if (*error_status)
        return false;

    fill->set_source_range(TimeRange(
        RationalTime(0, end_of_current.rate()),
        place_range.start_time() - end_of_current
    ));

    int index = (int)track->children().size();
    stack->add_event(new InsertItemEdit(index++, track, fill));
    stack->add_event(new InsertItemEdit(index, track, item));
    return true;
}


/*
    Utility for generating a "duplicate" of an item when another item is
    being inserted and the existing item contains the new.

    Given the scenario:

            [   B    ]
            ↓
    +--------------------------+
    |[            A           ]|
    +--------------------------+

    This will produce the following:

    +--------------------------+
    |[  A  ][   B    ][   A'  ]|
    +--------------------------+

    Where A' is a clone of the original A. Both A and A' have their source
    ranges modified to to retain the original begin and finish points.
*/
bool slice_and_gen(int insertion_index,
                   Track* track,
                   EventStack *stack,
                   const Intersection &intersection,
                   ErrorStatus *error_status)
{
    stack->add_event(new ModifyItemSourceRangeEdit(
            intersection.item,
            intersection.source_range_before
    ));

    // TODO: This will need massaging for ID system and naming
    // FIXME: Do we also need to be careful with effects and transitions?
    Item *duplicate = dynamic_cast<Item*>(intersection.item->clone(error_status));
    if (*error_status || !duplicate)
        return false;

    stack->add_event(new InsertItemEdit(insertion_index, track, duplicate));
    stack->add_event(new ModifyItemSourceRangeEdit(
            duplicate,
            intersection.source_range_after
    ));
    return true;
}

} // edit_util


Intersections get_intersections(
        Track *track,
        TimeRange const& track_range,
        ErrorStatus* error_status,
        int count)
{
    std::vector<RetainedComposable> const& children = track->children();
    Intersections intersections;
    bool contact = false;

    for (int idx = 0; idx < children.size(); idx++) {
        RetainedComposable const& composable = children[idx];

        // FIXME: Handle transitions
        Item *child = dynamic_cast<Item *>(composable.value);
        if (!child)
            continue;

        // Coords relative to the track
        auto child_range = track->range_of_child_at_index(idx, error_status);

        // Coords relative to the child
        auto child_source_range = child->source_range().value();

        if (*error_status) {
            return {};
        }

        if (track_range.contains(child_range)) {
            /*
                This child is completely covered by the track_range. No source_before or source_after
                will be produced. Handles four cases:

                [   track_range    ]        - child_range start is after track_range start
                  [ child_range ]           - child_range end is before the track_range end

                      - OR -
                
              begins
                ↓
                [   track_range    ]        - child_range start begins track_range
                [ child_range  ]            - child_range end is before track_range end

                      - OR -

                                finishes
                                   ↓
                [   track_range    ]        - child_range start is after is the track_range start
                    [ child_range  ]        - child_range end finishes track_range

                      - OR -

              begins            finishes
                ↓                  ↓
                [   track_range    ]        - child_range start begins track_range
                [   child_range    ]        - child_range end finishes track_range
            */

            contact = true;
            intersections.push_back({
                Intersection::Contains,
                child,
                idx
            });

        } else if (child_range.contains(track_range)) {

            /*
                This child completely covers the track_range. Handle three cases

            source_before        source_after
                 ↓                    ↓
                | |[   track_range  ]| |    - child_range start is before track_range start
                [    child_range       ]    - child_range end is after track_range end

                      - OR -

              begins        source_after
                ↓                 ↓
                [  track_range  ]| |        - child_range start begins track_range
                [   child_range    ]        - child_range end is after track_range end

                      - OR -

            source_before        finishes
                 ↓                  ↓
                | |[  track_range   ]       - child_range start is before track_range
                [      child_range  ]       - child_range end finishes track_range
            */

            contact = true;
            Intersection::IntersectionType type = Intersection::Contained;

            TimeRange source_before = TimeRange(
                child_source_range.start_time(),
                (track_range.start_time() - child_range.start_time())
            );
            TimeRange source_after = TimeRange(
                child_source_range.start_time() + (track_range.end_time_exclusive() - child_range.start_time()),
                (child_range.end_time_exclusive() - track_range.end_time_exclusive())
            );

            if (child_range.begins(track_range.start_time())) {
                // child_range.finishes(...) means Contains which is handled above
                type = Intersection::IntersectBefore;
            }
            else if (track_range.finishes(child_range)) {
                type = Intersection::IntersectAfter;
            }

            intersections.push_back({
                type,
                child,
                idx,
                source_before,
                source_after
            });

        } else if (track_range.intersects(child_range)) {
            /*
                The track range intersects the child range in some way. This covers the rest
                of the cases _except_ for the case of Intersection::None

                              source_after
                                  ↓
                [  track_range  ]| |      - child_range start is after track_range start
                    [  child_range ]         and before track_range end
                                          - child_range end is after track_range
                      - OR -

            source_before
                 ↓
                | |[  track_range  ]       - child_range start is 
                [  child_range   ]
            */
            contact = true;
            
            Intersection::IntersectionType type;
            TimeRange source_before, source_after;

            if (track_range.start_time() < child_range.start_time()) {
                // The new range starts _before_ this child
                type = Intersection::IntersectBefore;
                source_after = TimeRange(
                    child_source_range.start_time() + (track_range.end_time_exclusive() - child_range.start_time()),
                    (child_range.end_time_exclusive() - track_range.end_time_exclusive())
                );

            } else {
                // .contains() above takes care of other cases
                type = Intersection::IntersectAfter;
                source_before = TimeRange(
                    child_source_range.start_time(),
                    (track_range.start_time() - child_range.start_time())
                );
            }

            intersections.push_back({
                type,
                child,
                idx,
                source_before,
                source_after
            });

        } else {
            /*
                When no intersections are found. This handles all other cases

                [ track_range ]    [  child_range  ]
                [ child_range ]    [  track_range  ]

                             meets
                               ↓
                [ track_range ][ child_range ]
                [ child_range ][ track_range ]
            */
            if (contact)
                break; // Passed all intersections

        }

        if (contact && count > 0 && --count == 0) {
            break; // We have all we care to look for
        }
    }
    return intersections;
}

/* -------------------------------------------------------------------------- */

EventStack* overwrite(Item* item,
                      Track* track,
                      optional<RationalTime> const& track_time,
                      ErrorStatus *error_status,
                      Item* fill_template,
                      bool preview)
{
    if (!item) {
        *error_status = ErrorStatus::NOT_AN_ITEM;
        return nullptr;
    }

    auto item_range = item->source_range();
    if (!item_range) {
        *error_status = ErrorStatus::INVALID_TIME_RANGE;
        return nullptr;
    }

    TimeRange place_range = TimeRange(track_time.value(), (*item_range).duration());
    Intersections intersections = get_intersections(track, place_range, error_status);
    if (*error_status)
        return nullptr;

    std::unique_ptr<EventStack> stack(new EventStack({}, "overwrite"));

    if (intersections.size() == 0) {
        //
        // Use the fill template (or Gap) to pad the track with something until
        // this item is meant to appear
        //
        bool ok = edit_util::push_fill(
            stack.get(), track, item, fill_template, place_range, error_status
        );
        if (!ok)
            return nullptr;
    }
    else {
        int insertion_index = -1;

        for (const Intersection &intersection: intersections) {

            if (insertion_index < 0)
                insertion_index = intersection.index;

            switch (intersection.type)
            {
            case Intersection::Contains:
            {
                // Any time we cover the entire range of another item, we replace
                // it completely
                stack->add_event(new RemoveItemEdit(intersection.item));
                break;
            }
            case Intersection::Contained:
            {
                // In this event, there should be both a source_before and source_after
                // Therefore, we must duplicate the current item, adjust the SR of the
                // initial, and insert it's duplicate after the up-coming item
                insertion_index++; // New item comes _after_ the intersected item
                bool ok = edit_util::slice_and_gen(
                    insertion_index,
                    track,
                    stack.get(),
                    intersection,
                    error_status
                );
                if (!ok)
                    return nullptr;
                break;
            }
            case Intersection::IntersectBefore:
            {
                // In this instance, we don't remove the item, we only adjust the
                // source range to be the source_after
                stack->add_event(new ModifyItemSourceRangeEdit(
                    intersection.item,
                    intersection.source_range_after
                ));
                break;
            }
            case Intersection::IntersectAfter:
            {
                insertion_index++; // New item comes _after_ the intersected item
                stack->add_event(new ModifyItemSourceRangeEdit(
                    intersection.item,
                    intersection.source_range_before
                ));
                break;
            }
            case Intersection::None:
                break;
            }
        }

        // We'll always insert the item for overwrite
        stack->add_event(new InsertItemEdit(insertion_index, track, item));
    }

    if (!preview) {
        stack->run(error_status);
        if (*error_status) {
            return nullptr;
        }
    }
    return stack.release();
}



EventStack* insert(Item* item,
                   Track* track,
                   optional<RationalTime> const& track_time,
                   ErrorStatus *error_status,
                   Item* fill_template,
                   bool preview)
{
    if (!item) {
        *error_status = ErrorStatus::NOT_AN_ITEM;
        return nullptr;
    }

    auto item_range = item->source_range();
    if (!item_range) {
        *error_status = ErrorStatus::INVALID_TIME_RANGE;
        return nullptr;
    }

    auto ttime = track_time.value();
    TimeRange place_range = TimeRange(ttime, RationalTime(1, ttime.rate()));

    Intersections intersections = get_intersections(
        track, place_range, error_status, /*count=*/1
    );
    if (*error_status)
        return nullptr;

    std::unique_ptr<EventStack> stack(new EventStack({}, "insert"));

    if (intersections.size() == 0) {
        bool ok = edit_util::push_fill(
            stack.get(), track, item, fill_template, place_range, error_status
        );
        if (!ok)
            return nullptr;

    } else {

        const Intersection &intersection = intersections[0];
        int insertion_index = intersection.index;

        switch (intersection.type)
        {
        case Intersection::Contained:
        case Intersection::IntersectAfter:
        {
            // We have to split an item, and shift everything else
            insertion_index++;
            bool ok = edit_util::slice_and_gen(
                insertion_index,
                track,
                stack.get(),
                intersection,
                error_status
            );
            if (!ok)
                return nullptr;
            break;
        }
        case Intersection::Contains:
        case Intersection::IntersectBefore:
        case Intersection::None:
            break;
        }

        // We'll always insert the item
        stack->add_event(new InsertItemEdit(insertion_index, track, item));
    }

    if (!preview) {
        stack->run(error_status);
        if (*error_status) {
            return nullptr;
        }
    }
    return stack.release();
}


EventStack* slice(Item* item,
                  optional<RationalTime> const& at_time,
                  ErrorStatus* error_status,
                  Coordinates coordinates,
                  bool preview)
{
    if (!item) {
        *error_status = ErrorStatus::NOT_AN_ITEM;
        return nullptr;
    }

    Track* use_track = dynamic_cast<Track*>(item->parent());
    if (!use_track) {
        *error_status = {ErrorStatus::NOT_A_CHILD_OF, "cannot slice trackless item"};
		return nullptr;
    }

    auto item_range = use_track->range_of_child(item, error_status);
    if (*error_status) {
        return nullptr;
    }

    std::unique_ptr<EventStack> stack(new EventStack({}, "slice"));
    RationalTime use_time = at_time.value();
    //
    // Convert all coordinated to the track (Coordinates::Parent) time
    // so we can build an intersection (f any)
    //
    switch (coordinates)
    {
    case Coordinates::Item:
    {
        use_time = item_range.start_time() + (item_range.duration() - use_time);
        break;
    }
    case Coordinates::Global:
    {
        // TODO - find based on the global timeline...
        break;
    }
    case Coordinates::Parent:
    default:
        break;
    }

    RationalTime one_frame = RationalTime(1, use_time.rate());
	TimeRange intersect_range = TimeRange(use_time, one_frame);

    // Account for begins and finishes
    if (intersect_range.begins(item_range) || item_range.finishes(intersect_range.end_time_exclusive())) {
        return stack.release();
    }

    auto intersections = get_intersections(
        use_track, intersect_range, error_status, /*count=*/1
    );
    if (intersections.size() == 0) {
		return stack.release(); // No slice to be found
    }

	Intersection &intersection = intersections.front();
    bool ok = edit_util::slice_and_gen(
        intersection.index,
        use_track,
        stack.get(),
        intersection,
        error_status
    );
    if (!ok) {
        return nullptr;
    }
	if (!preview) {
		stack->run(error_status);
		if (*error_status) {
			return nullptr;
		}
	}
    return stack.release(); // We've accomplished the slice
}


} }
