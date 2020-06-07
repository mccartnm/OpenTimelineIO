set (OTIO_EVENT_HEADERS
     event/event.h
     event/eventType.h
     event/registry.h
     event/eventStack.h)

set (OTIO_EVENT_SOURCES
     event/event.cpp
     event/eventType.cpp
     event/registry.cpp
     event/eventStack.cpp
     ${OTIO_EVENT_HEADERS}
     PARENT_SCOPE)

if (NOT OTIO_CXX_NOINSTALL)
    install(FILES ${OPENTIME_HEADER_FILES} DESTINATION include/opentimelinio/event)
endif (NOT OTIO_CXX_NOINSTALL)
