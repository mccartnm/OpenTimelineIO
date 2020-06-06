set(ALGOS_HEADER_FILES
    version.h
    errorStatus.h
    editEvent.h
    editAlgorithms.h
    )

set (ALGOS_SOURCE_FILES
     openedit SHARED
     errorStatus.cpp
     editAlgorithms.cpp
     editEvent.cpp
     ${ALGOS_HEADER_FILES}
     # Access Above
     PARENT_SCOPE)


# target_include_directories(openedit PUBLIC
#                            "${PROJECT_SOURCE_DIR}/src"
#                            "${PROJECT_SOURCE_DIR}/src/deps"
#                            "${PROJECT_SOURCE_DIR}/src/deps/optional-lite/include")

# target_link_libraries(openedit PUBLIC opentime opentimelineio)
# set_target_properties(openedit PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
#             MACOSX_RPATH true WINDOWS_EXPORT_ALL_SYMBOLS true)

# install(TARGETS openedit
#         RUNTIME DESTINATION bin
#         LIBRARY DESTINATION lib)

if (NOT OTIO_CXX_NOINSTALL)
    install(FILES ${ALGOS_HEADER_FILES} DESTINATION include/opentimelineio/algos)
endif (NOT OTIO_CXX_NOINSTALL)
