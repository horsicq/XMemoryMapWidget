include_directories(${CMAKE_CURRENT_LIST_DIR})

if (NOT DEFINED XFORMATS_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../Formats/xformats.cmake)
    set(XMEMORYMAPWIDGET_SOURCES ${XMEMORYMAPWIDGET_SOURCES} ${XFORMATS_SOURCES})
endif()
if (NOT DEFINED XLINEEDITHEX_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../Controls/xlineedithex.cmake)
    set(XMEMORYMAPWIDGET_SOURCES ${XMEMORYMAPWIDGET_SOURCES} ${XLINEEDITHEX_SOURCES})
endif()

set(XMEMORYMAPWIDGET_SOURCES
    ${XMEMORYMAPWIDGET_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/dialogmemorymap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dialogmemorymap.h
    ${CMAKE_CURRENT_LIST_DIR}/dialogmemorymap.ui
    ${CMAKE_CURRENT_LIST_DIR}/xmemorymapwidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmemorymapwidget.h
    ${CMAKE_CURRENT_LIST_DIR}/xmemorymapwidget.ui
)
