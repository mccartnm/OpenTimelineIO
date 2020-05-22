pybind11_add_module(_openedit
                    openedit_bindings.cpp
                    opentime_rationalTime.cpp
                    opentime_timeRange.cpp
                    opentime_timeTransform.cpp
                    opentime_bindings.h)

target_include_directories(_openedit PUBLIC pybind11/include)
target_link_libraries(_openedit PUBLIC opentimelineio opentime openedit)
set_target_properties(_openedit PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

if (OTIO_PYTHON_OTIO_DIR)
    install(TARGETS _openedit DESTINATION "${OTIO_PYTHON_OTIO_DIR}")
endif (OTIO_PYTHON_OTIO_DIR)




