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

/*
    Obtain the Intersections of a given time range on a track. This is excellent for
    edit commands as it can define the items affected by a time range which is the
    most common form of coordinates from an editors perspective.

    There are thirteen scenarios that can occur between two time ranges in terms of
    intersections.

    The Intersection object has a source_before and source_after term which depict
    the remainder time range of the active track items. See the below examples for
    more information.

    @note: `source_range_before` and `source_range_after` are in the item coordinates
           not the track.

    If the track_range completely covers the range of a child, there is no source_after
    or source_begin.

    If the track_range begins or finishes the range of a child, there is no source_before
    or source_after respectively.
*/
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

source_before ->|  |                 | |<- source_after
                   [   track_range  ]       - child_range start is before track_range start
                [    child_range      ]     - child_range end is after track_range end

                      - OR -

              begins
                ↓                |  |<- source_after
                [  track_range  ]           - child_range start begins track_range
                [   child_range     ]       - child_range end is after track_range end

                      - OR -

                                 finishes
source_before ->|  |                ↓
                   [  track_range   ]       - child_range start is before track_range
                [      child_range  ]       - child_range end finishes track_range
            */

            contact = true;
            Intersection::IntersectionType type = Intersection::Contained;

            // WARNING:
            // This is iffy at best. Needs proofing
            //
            TimeRange source_before = TimeRange(
                child_source_range.start_time(),
                (track_range.start_time() - child_range.start_time())
            );
            TimeRange source_after = TimeRange(
                child_source_range.start_time() + (track_range.end_time_exclusive() - child_range.start_time()),
                (child_range.end_time_exclusive() - track_range.end_time_exclusive())
            );

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
                idx,
                source_before,
                source_after
            });

        } else if (track_range.overlaps(child_range)) {
            /*
                The track range overlaps the child range in some way. This covers the rest
                of the cases _except_ for the case of Intersection::None

                                 |    |<- source_after
                [  track_range  ]           - child_range start is after track_range start
                    [    child_range  ]         and before track_range end
                                            - child_range end is after track_range
                      - OR -

source_before ->|   |
                    [  track_range  ]       - child_range start is 
                [  child_range   ]
            */
            contact = true;
            
            Intersection::IntersectionType type;
            TimeRange source_before, source_after;

            if (track_range.start_time() < child_range.start_time()) {
                // The new range starts _before_ this child
                type = Intersection::OverlapBefore;
                source_after = TimeRange(
                    child_source_range.start_time() + (track_range.end_time_exclusive() - child_range.start_time()),
                    (child_range.end_time_exclusive() - track_range.end_time_exclusive())
                );

            } else {
                // .contains() above takes care of other cases
                type = Intersection::OverlapAfter;
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
        // Short circuit means of using the fill template to 
        // TODO
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
                stack->add_event(new ModifyItemSourceRangeEdit(
                        intersection.item,
                        intersection.source_range_before
                ));

                // TODO: This will need massaging for ID system and naming
                // FIXME: Do we also need to be careful with effects and transitions?
                Item *duplicate = dynamic_cast<Item*>(intersection.item->clone(error_status));
                if (*error_status || !duplicate)
                    return nullptr;

                stack->add_event(new InsertItemEdit(insertion_index, track, duplicate));
                stack->add_event(new ModifyItemSourceRangeEdit(
                        duplicate,
                        intersection.source_range_after
                ));
                break;
            }
            case Intersection::OverlapBefore:
            {
                // In this instance, we don't remove the item, we only adjust the
                // source range to be the source_after
                stack->add_event(new ModifyItemSourceRangeEdit(
                    intersection.item,
                    intersection.source_range_after
                ));
                break;
            }
            case Intersection::OverlapAfter:
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

} }
