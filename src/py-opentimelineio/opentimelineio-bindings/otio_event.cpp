#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>

#include "opentimelineio/event/event.h"
#include "opentimelineio/event/eventStack.h"

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
        });
}


void otio_event_bindings(py::module m) {
    define_event(m);
}
