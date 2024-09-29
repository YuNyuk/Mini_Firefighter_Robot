QT       += core gui network widgets

TEMPLATE = app
TARGET = FireRobot
INCLUDEPATH += .

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/main.cpp \
    src/mainwidget.cpp \
    src/rosnode.cpp \
    src/server.cpp \
    src/tab1camera.cpp \ \
    src/tab2roscontrol.cpp \
    src/tab3mapping.cpp

HEADERS += \
    include/fire_robot/mainwidget.h \
    include/fire_robot/rosnode.h \
    include/fire_robot/server.h \
    include/fire_robot/tab1camera.h \
    include/fire_robot/tab2roscontrol.h \ \
    include/fire_robot/tab3mapping.h

FORMS += \
    ui/mainwidget.ui \
    ui/tab1camera.ui \
    ui/tab2roscontrol.ui \
    ui/tab3mapping.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += /usr/include/opencv4/
#LIBS += `pkg-config opencv4 --cflags --libs`
