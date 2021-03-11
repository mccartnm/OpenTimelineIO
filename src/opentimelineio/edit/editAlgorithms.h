#pragma once

#include "opentime/rationalTime.h"
#include "opentimelineio/item.h"
#include "opentimelineio/track.h"
#include "opentimelineio/edit/edit.h"
#include "opentimelineio/event/eventStack.h"

namespace opentimelineio { namespace OPENTIMELINEIO_VERSION {

using RetainedItem = SerializableObject::Retainer<Item>;
using RetainedComposition = SerializableObject::Retainer<Composition>;
using RetainedComposable = SerializableObject::Retainer<Composable>;
using RetainedEvent = SerializableObject::Retainer<Event>;

/*
    An intersection utility with ranges that correspond based
    on the IntersectionType.
*/
struct Intersection
{
    enum IntersectionType {
        None = -1,          //< covers "all before", "all after", and "meets"
        Contains,           //< range contains other item
        Contained,          //< range is contained by an item
        IntersectBefore,    //< range intersects item and (starts before or begins)
        IntersectAfter,     //< range intersects item and (ends after or finishes)
    };

    IntersectionType type;
    Item *item;
    int index;
    optional<TimeRange> source_range_before;
    optional<TimeRange> source_range_after;

    Intersection(
        IntersectionType t,
        Item *i,
        int idx,
        optional<TimeRange> source_before = nullopt,
        optional<TimeRange> source_after = nullopt)
        : type(t)
        , item(i)
        , index(idx)
        , source_range_before(source_before)
        , source_range_after(source_after)
        {}

    ~Intersection() {}
};
using Intersections = std::vector<Intersection>;


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
        Track* track,
        TimeRange const& track_range,
        ErrorStatus* error_status,
        int count = -1);


/**
 * Overwrite items on a track with another item. This will remove, trim, or
 * fill where required to completely set the item at the given track_time
 *
 *   let item = C
 *   let track_time = 10 @ 24
 *
 *   ---
 *   1 - Item overlaps "GAP" and "A", contains "B"
 *
 *           [0        C         40]
 *   0       |                                     N
 *   | ------------------------------------------- |
 *   | [0    GAP  20][0  B  10][0     A    30]     |
 *   | ------------------------------------------- |
 *   
 *  RESULT
 *      - "C" inserted after "GAP"
 *      - "B" Removed
 *      - "GAP" duration changed
 *      - "A" source start + duration changed
 *   | ------------------------------------------- |
 *   | [GAP ][0       C        40][10  A  30]      |
 *   | ------------------------------------------- |
 *
 *   ---
 *   2 - Item is contained by track item "A"
 *
 *           [0        C         40]
 *   0       |                                     N
 *   | ------------------------------------------- |
 *   | [0               A                50]       |
 *   | ------------------------------------------- |
 *
 *   RESULT
 *       - "A" split at track_time (See documentation below for procedure)
 *       - "A'" source_range start = "A" source_range end + "C" duration
 *       - "C" inserted after "A"
 *   | ------------------------------------------- |
 *   | [ A 10][0        C         40][40 A' ]      |
 *   | ------------------------------------------- |
 *
 *   ---
 *   3 - Item is passed tracks current duration
 *           [0        C         40]
 *   0       |                                     N
 *   | ------------------------------------------- |
 *   | <nothing>                                   |
 *   | ------------------------------------------- |
 *
 *   RESUT
 *       - Fill template cloned to fill empty space
 *   | ------------------------------------------- |
 *   | [ FILL ][0        C         40]             |
 *   | ------------------------------------------- |
 *
 * @param item
 * @param track
 * @param track_time
 * @param error_status
 * @param fill_template
 * @param preview
 */
EventStack* overwrite(Item* item,
                      Track* track,
                      optional<RationalTime> const& track_time,
                      ErrorStatus* error_status,
                      Item* fill_template = nullptr,
                      bool preview = false);



/**
 * Insert an item at a given time. This will slice and push items
 * as required to make sure the item starts at the specified time.
 * No content is removed.
 *
 *    let item = C
 *    let track_time = 10 @ 24
 *
 *    1 - Item overlaps "GAP" and "A", contains "B"
 *
 *            [0        C         40]
 *    0       |                                     N
 *    | ------------------------------------------- |
 *    | [0    GAP  20][0  B  10][0     A    30]     |
 *    | ------------------------------------------- |
 *    
 *    RESULT
 *        - "GAP" split at track_time
 *        - Existing "GAP" duration reduced
 *        - New "GAP" duration adjusted to fit the rest
 *        - "C" inserted into track children after existing GAP
 *        - "B" and "A" left unchanged
 *    | ------------------------------------------- |
 *    | [GAP ][0       C        40][10 GAP 20][0    B ... <continued>
 *    | ------------------------------------------- |
 *
 *
 *    2 - Item is passed tracks current duration (eqiv to overwrite)
 *            [0        C         40]
 *    0       |                                     N
 *    | ------------------------------------------- |
 *    | <nothing>                                   |
 *    | ------------------------------------------- |
 *
 *    RESUT - Fill template cloned to fill empty space
 *    | ------------------------------------------- |
 *    | [ FILL ][0        C         40]             |
 *    | ------------------------------------------- |
 *
 * @param item
 * @param track
 * @param track_time
 * @param error_status
 * @param fill_template
 * @param preview
 */
EventStack* insert(Item* item,
                   Track* track,
                   optional<RationalTime> const& track_time,
                   ErrorStatus* error_status,
                   Item* fill_template = nullptr,
                   bool preview = false);



/**
 * Slice an item in two at a given time. This also takes a coordinate
 * enum to know the reference point of the item
 *
 *    Let item = A
 *    let at_time = 25 @ 24fps
 *
 *    0                                             N
 *    | ------------------------------------------- |
 *    | [0    GAP  20][0         A           50]    |
 *    | ------------------------------------------- |
 *
 *    if coordinates == Local:
 *    | ------------------------------------------- |
 *    | [0    GAP  20][0    A   25][26   A  50]     |
 *    | ------------------------------------------- |
 *
 *    if coordinates == Parent:
 *    | ------------------------------------------- |
 *    | [0    GAP  20][0 A 5][6      A      50]     |
 *    | ------------------------------------------- |
 *
 *    if coordinates == Global:
 *        - This only matters if the parent is a track in a compound
 *          stack, at which point we might be better off having the
 *          user just convert from one time to the other
 *
 * In the event that no slice would happen (e.g. at_time is outside of the)
 * items scope, an empty stack is returned
 *
 * @param item
 * @param at_timek
 * @param coordinates
 * @param error_status
 * @param preview
 */
EventStack* slice(Item* item,
                  optional<RationalTime> const& at_time,
                  ErrorStatus* error_status,
                  Coordinates coordinates = Coordinates::Parent,
                  bool preview = false);

} }
