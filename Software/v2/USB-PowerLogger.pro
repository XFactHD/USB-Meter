QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS QCUSTOMPLOT_USE_LIBRARY

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    hidhandler.cpp \
    main.cpp \
    mainwindow.cpp \
    qhidapi/qhidapi.cpp \
    qhidapi/qhidapi_p.cpp \
    qhidapi/hid.c

HEADERS += \
    commhandler.h \
    hidhandler.h \
    mainwindow.h \
    qhidapi/hidapi.h \
    qhidapi/qhidapi.h \
    qhidapi/qhidapi_global.h \
    qhidapi/qhidapi_p.h \
    qhidapi/qhiddeviceinfo.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CONFIG(debug, release|debug) {
  win32:QCPLIB = qcustomplotd2
  else: QCPLIB = qcustomplotd
} else {
  win32:QCPLIB = qcustomplot2
  else: QCPLIB = qcustomplot
}
LIBS += -L./ -l$$QCPLIB
LIBS += -lSetupapi

#RC_ICONS = "icon.ico"
