QT       += core gui
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += VENTSIM

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../ArduinoVent/alarm.cpp \
    ../ArduinoVent/breather.cpp \
    ../ArduinoVent/crc.cpp \
    ../ArduinoVent/event.cpp \
    ../ArduinoVent/log.cpp \
    ../ArduinoVent/pressure.cpp \
    ../ArduinoVent/properties.cpp \
    ../ArduinoVent/ui_native.cpp \
    ../ArduinoVent/vent.cpp \
    hal_sim.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    ../ArduinoVent/alarm.h \
    ../ArduinoVent/breather.h \
    ../ArduinoVent/config.h \
    ../ArduinoVent/crc.h \
    ../ArduinoVent/event.h \
    ../ArduinoVent/hal.h \
    ../ArduinoVent/languages.h \
    ../ArduinoVent/log.h \
    ../ArduinoVent/pressure.h \
    ../ArduinoVent/properties.h \
    ../ArduinoVent/ui_native.h \
    ../ArduinoVent/vent.h \
    mainwindow.h

FORMS += \
    mainwindow.ui
    
INCLUDEPATH += ../ArduinoVent/

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    VentResource.qrc

DISTFILES +=
