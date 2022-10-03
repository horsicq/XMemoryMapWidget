INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/dialogmemorymap.h \
    $$PWD/xmemorymapwidget.h

SOURCES += \
    $$PWD/dialogmemorymap.cpp \
    $$PWD/xmemorymapwidget.cpp

FORMS += \
    $$PWD/dialogmemorymap.ui \
    $$PWD/xmemorymapwidget.ui

!contains(XCONFIG, xformats) {
    XCONFIG += xformats
    include($$PWD/../Formats/xformats.pri)
}

!contains(XCONFIG, xlineedithex) {
    XCONFIG += xlineedithex
    include($$PWD/../Controls/xlineedithex.pri)
}

!contains(XCONFIG, xhexview) {
    XCONFIG += xhexview
    include($$PWD/../XHexView/xhexview.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md \
    $$PWD/xmemorymapwidget.cmake
