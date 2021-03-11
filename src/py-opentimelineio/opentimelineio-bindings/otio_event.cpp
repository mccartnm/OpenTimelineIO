#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "opentimelineio/event/event.h"
#include "opentimelineio/event/eventStack.h"

#include "opentimelineio/edit/edit.h"
#include "opentimelineio/edit/insertItemEdit.h"
#include "opentimelineio/edit/removeItemEdit.h"
#include "opentimelineio/edit/modifyItemSourceRange.h"

#include "opentimelineio/edit/editAlgorithms.h"

#include "otio_errorStatusHandler.h"
#include "otio_utils.h"

namespace py = pybind11;
using SOWithMetadata = SerializableObjectWithMetadata;

static void define_event(py::module m) {
    py::class_<Event, SOWithMetadata, managing_ptr<Event>>(m, "Event", py::dynamic_attr())
        .def(py::init([](std::string const& name,
                         py::object metadata) {
                            return new Event(name,
                                             py_to_any_dictionary(metadata)); }),
             name_arg,
             metadata_arg)
        .def("run", [](Event *event) {
            event->run(ErrorStatusHandler());
        })
        .def("revert", [](Event *event) {
            event->revert(ErrorStatusHandler());
        });

    py::class_<EventStack, Event, managing_ptr<EventStack>>(m, "EventStack", py::dynamic_attr())
        .def(py::init([](std::string name, py::object events, py::object metadata) {
                            return new EventStack(py_to_vector<Event*>(events),
                                                  name,
                                                  py_to_any_dictionary(metadata)); }),
            name_arg,
            "events"_a = py::none(),
            metadata_arg)
        // .def_property("events", &EventStack::events, &EventStack::set_events)
        .def("add_event", [](EventStack *stack, Event* event) {
            stack->add_event(event);
        }, "event"_a);
}


static void define_edit_events(py::module m) {

    py::enum_<Coordinates>(m, "Coordinates")
        .value("Item", Coordinates::Item)
        .value("Parent", Coordinates::Parent)
        .value("Global", Coordinates::Global);


    py::class_<ItemEdit, Event, managing_ptr<ItemEdit>>(m, "ItemEdit", py::dynamic_attr())
        .def(py::init([](std::string const& name, Track* track, Item* item, py::object metadata) {
                            return new ItemEdit(track, item, name,
                                                py_to_any_dictionary(metadata)); }),
            name_arg,
            "track"_a = nullptr,
            "item"_a = nullptr,
            metadata_arg)
        .def_property_readonly("track", &ItemEdit::track)
        .def_property_readonly("item", &ItemEdit::item);


    // -------------------------------------------------------------------- INSERT
    py::class_<InsertItemEdit, ItemEdit, managing_ptr<InsertItemEdit>>(m, "InsertItemEdit", py::dynamic_attr())
        .def(py::init([](std::string const& name, int index, Track* track, Item* item, py::object metadata) {
                            return new InsertItemEdit(index, track, item, name,
                                                      py_to_any_dictionary(metadata)); }),
            name_arg,
            "index"_a = 0,
            "track"_a = nullptr,
            "item"_a = nullptr,
            metadata_arg);

    // -------------------------------------------------------------------- REMOVE
    py::class_<RemoveItemEdit, ItemEdit, managing_ptr<RemoveItemEdit>>(m, "RemoveItemEdit", py::dynamic_attr())
        .def(py::init([](std::string const& name, Item* item, py::object metadata) {
                            return new RemoveItemEdit(item, name,
                                                      py_to_any_dictionary(metadata)); }),
            name_arg,
            "item"_a = nullptr,
            metadata_arg);

    // -------------------------------------------------------------------- MODIFY SOURCE
    py::class_<ModifyItemSourceRangeEdit, ItemEdit, managing_ptr<ModifyItemSourceRangeEdit>>(m, "ModifyItemSourceRangeEdit", py::dynamic_attr())
        .def(py::init([](std::string const& name,
                         Item* item,
                         optional<TimeRange> const& new_range,
                         py::object metadata) {
                            return new ModifyItemSourceRangeEdit(
                                item, new_range, name, py_to_any_dictionary(metadata)
                            ); }),
            name_arg,
            "item"_a = nullptr,
            "new_range"_a = nullopt,
            metadata_arg);
}


static void define_edit_commands(py::module m) {

    auto intersect = py::class_<Intersection>(m, "Intersection");

    // Build an enum for the types of intersections
    py::enum_<Intersection::IntersectionType>(intersect, "Type")
        .value("None", Intersection::IntersectionType::None)
        .value("Contains", Intersection::IntersectionType::Contains)
        .value("Contained", Intersection::IntersectionType::Contained)
        .value("IntersectBefore", Intersection::IntersectionType::IntersectBefore)
        .value("IntersectAfter", Intersection::IntersectionType::IntersectAfter);

    intersect
        .def_readonly("type", &Intersection::type)
        .def_readonly("item", &Intersection::item)
        .def_readonly("index", &Intersection::index)
        .def_readonly("source_range_before", &Intersection::source_range_before)
        .def_readonly("source_range_after", &Intersection::source_range_after)
        .def_static("get_intersections", [](Track *track, TimeRange const& track_range, int count) {
                                            return get_intersections(
                                                track,
                                                track_range,
                                                ErrorStatusHandler(),
                                                count
                                            );
                                        },
            "track"_a,
            "track_range"_a,
            "count"_a = -1
        );


    m.def("overwrite", [](Item *item,
                          Track *track,
                          RationalTime track_time,
                          Item *fill_template,
                          bool preview) {
                            return overwrite(
                                item,
                                track,
                                track_time,
                                ErrorStatusHandler(),
                                fill_template,
                                preview
                            );
                            },
            "item"_a,
            "track"_a,
            "track_time"_a,
            "fill_template"_a = nullptr,
            "preview"_a = false);


    m.def("insert", [](Item *item,
                       Track *track,
                       RationalTime track_time,
                       Item *fill_template,
                       bool preview) {
                         return insert(
                             item,
                             track,
                             track_time,
                             ErrorStatusHandler(),
                             fill_template,
                             preview
                         );
                         },
            "item"_a,
            "track"_a,
            "track_time"_a,
            "fill_template"_a = nullptr,
            "preview"_a = false);


    m.def("slice_item", [](Item *item,
                           RationalTime at_time,
                           Coordinates coordinates,
                           bool preview) {
                            return slice(
                                item,
                                at_time,
                                ErrorStatusHandler(),
                                coordinates,
                                preview
                            );
                            },
            "item"_a,
            "at_time"_a,
            "coordinates"_a = Coordinates::Parent,
            "preview"_a = false);
}


void otio_event_bindings(py::module m) {
    define_event(m);
    define_edit_events(m);
    define_edit_commands(m);
}
