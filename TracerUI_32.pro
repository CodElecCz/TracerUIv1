#-------------------------------------------------
#
# Project created by QtCreator 2017-12-20T12:57:57
#
#-------------------------------------------------

QT       += core gui xml charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TracerUI
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    MainWindow.cpp \
    NodeDialog.cpp \
    TreeItem.cpp \
    TreeView.cpp \
    LineEditDelegate.cpp \
    MainWindowGraph.cpp \
    MainWindowTrace.cpp \
    ModelELM327.cpp \
    ModelOMC8000.cpp \
    ModelModbus.cpp
    
HEADERS += \
    TreeView.h \
    TreeItem.h \
    MainWindow.h \
    NodeDialog.h \
    LineEditDelegate.h \
    ModelModbus.h \
    ModelELM327.h \
    ModelOMC8000.h

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../Lib/Build/release/ -lModbus -lOMC8000 -lELM327 -lSharedNodes
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Lib/Build/debug/ -lModbus -lOMC8000 -lELM327 -lSharedNodes

INCLUDEPATH += $$PWD/../../Lib/
INCLUDEPATH += $$PWD/../../Lib/Build/

DEPENDPATH += $$PWD/../../Lib/
DEPENDPATH += $$PWD/../../Lib/Build/

RESOURCES += \
    TracerUI.qrc

FORMS += \
 TreeView.ui \
    NodeDialog.ui \
    MainWindow.ui \
    AboutDialog.ui

RC_FILE += \
    Resource.rc


