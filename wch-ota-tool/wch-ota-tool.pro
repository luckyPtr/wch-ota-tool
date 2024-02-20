QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        apptask.cpp \
        ble.cpp \
        main.cpp \
        ota.cpp


LIBS+=-L$$PWD/./ -lWCHBLEDLL

HEADERS += \
    WCHBLEDLL.h \
    apptask.h \
    ble.h \
    ota.h

