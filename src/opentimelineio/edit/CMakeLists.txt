set(OTIO_EDIT_HEADER_FILES
    edit/edit.h
    edit/editAlgorithms.h
    )

set (OTIO_EDIT_SOURCES
     edit/edit.cpp
     ${OTIO_EDIT_HEADER_FILES}
     # Access Above
     PARENT_SCOPE)

if (NOT OTIO_CXX_NOINSTALL)
    install(FILES ${OTIO_EDIT_HEADER_FILES} DESTINATION include/opentimelineio/edit)
endif (NOT OTIO_CXX_NOINSTALL)
