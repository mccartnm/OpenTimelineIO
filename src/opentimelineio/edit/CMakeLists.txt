set(OTIO_EDIT_HEADER_FILES
    edit/edit.h
    edit/insertItemEvent.h
    PARENT_SCOPE
    )

set (OTIO_EDIT_SOURCES
     edit/edit.cpp
     edit/insertItemEvent.cpp
     ${OTIO_EDIT_HEADER_FILES}
     # Access Above
     PARENT_SCOPE)

