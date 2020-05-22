set(OPENEDIT_HEADER_FILES
    version.h
    errorStatus.h
    editEvent.h
    editAlgorithms.h
    )

add_library(openedit SHARED
            errorStatus.cpp
            editAlgorithms.cpp
            editEvent.cpp
            ${OPENEDIT_HEADER_FILES})

target_include_directories(openedit PUBLIC
                           "${PROJECT_SOURCE_DIR}/src"
                           "${PROJECT_SOURCE_DIR}/src/deps"
                           "${PROJECT_SOURCE_DIR}/src/deps/optional-lite/include")

target_link_libraries(openedit PUBLIC opentime opentimelineio)
set_target_properties(openedit PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
            MACOSX_RPATH true WINDOWS_EXPORT_ALL_SYMBOLS true)

install(TARGETS openedit
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

if (NOT OTIO_CXX_NOINSTALL)
    install(FILES ${OPENEDIT_HEADER_FILES} DESTINATION include/openedit)
endif (NOT OTIO_CXX_NOINSTALL)
