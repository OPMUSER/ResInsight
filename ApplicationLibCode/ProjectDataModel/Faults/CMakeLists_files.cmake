set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInView.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInViewCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultRASettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultRAPreprocSettings.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultRAPostprocSettings.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInView.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultInViewCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultRASettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultRAPreprocSettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaultRAPostprocSettings.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "ProjectDataModel\\Faults"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
