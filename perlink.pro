QT -= gui

CONFIG += c++11 console -Wunused-parameter
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += MEEIOT_SERVER

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        a_peer.cpp \
        main.cpp \
        peers.cpp \
        per_id.cpp \
        pipefiles.cpp \
        sock.cpp \
        sqliter.cpp \
        u_server.cpp \
        udp_xdea.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    a_peer.h \
    peers.h \
    per_id.h \
    pipefiles.h \
    sock.h \
    sqliter.h \
    u_server.h \
    udp_xdea.h



unix|win32: LIBS += -lsqlite3

DISTFILES +=
