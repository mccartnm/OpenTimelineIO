set (OTIO_EVENT_HEADER_FILES
     event/event.h
     event/registry.h
     event/eventStack.h
     PARENT_SCOPE)

set (OTIO_EVENT_SOURCES
     event/event.cpp
     event/registry.cpp
     event/eventStack.cpp
     ${OTIO_EVENT_HEADER_FILES}
     PARENT_SCOPE)
