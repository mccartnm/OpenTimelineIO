#pragma once

#include "opentime/rationalTime.h"
#include "opentimelineio/item.h"
#include "opentimelineio/track.h"
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
        OverlapBefore,      //< range overlaps item and (starts before or begins)
        OverlapAfter,       //< range overlaps item and (ends after or finishes)
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

Intersections get_intersections(
        Track *track,
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
                      ErrorStatus *error_status,
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
                   ErrorStatus *error_status,
                   Item* fill_template = nullptr,
                   bool preview = false);


} }
