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

!contains(XCONFIG, qhexview) {
    XCONFIG += qhexview
    include($$PWD/../QHexView/qhexview.pri)
}
