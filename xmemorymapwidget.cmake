# TODO guard
include_directories(${CMAKE_CURRENT_LIST_DIR})

include(${CMAKE_CURRENT_LIST_DIR}/../Formats/xformats.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../Controls/xlineedithex.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XHexView/xhexview.cmake)

set(XMEMORYMAPWIDGET_SOURCES
    ${XFORMATS_SOURCES}
    ${XLINEEDITHEX_SOURCES}
    ${XHEXVIEW_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/dialogmemorymap.ui
    ${CMAKE_CURRENT_LIST_DIR}/xmemorymapwidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/dialogmemorymap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xmemorymapwidget.cpp
)
