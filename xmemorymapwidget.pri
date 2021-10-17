INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

FORMS += \
    $$PWD/dialogmemorymap.ui \
    $$PWD/xmemorymapwidget.ui

HEADERS += \
    $$PWD/dialogmemorymap.h \
    $$PWD/xmemorymapwidget.h

SOURCES += \
    $$PWD/dialogmemorymap.cpp \
    $$PWD/xmemorymapwidget.cpp

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
    $$PWD/README.md
