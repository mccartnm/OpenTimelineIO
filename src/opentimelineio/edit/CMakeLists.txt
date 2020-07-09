set(OTIO_EDIT_HEADER_FILES
    edit/edit.h
    edit/insertItemEdit.h
    edit/removeItemEdit.h
    edit/modifyItemSourceRange.h
    PARENT_SCOPE)

set(OTIO_EDIT_SOURCES
    edit/edit.cpp
    edit/insertItemEdit.cpp
    edit/removeItemEdit.cpp
    edit/modifyItemSourceRange.cpp
    ${OTIO_EDIT_HEADER_FILES}
    # Access Above
    PARENT_SCOPE)

