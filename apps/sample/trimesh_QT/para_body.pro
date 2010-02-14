# Base options
TEMPLATE = app
LANGUAGE  = C++

# QT modules
QT += opengl

# Executable name
TARGET = para_body

# Directories
DESTDIR = .
UI_DIR = build/ui
MOC_DIR = build/moc
OBJECTS_DIR = build/obj

# Lib headers
INCLUDEPATH += .
INCLUDEPATH += ../../../../../vcglib

# Lib sources
SOURCES += ../../../../../vcglib/wrap/ply/plylib.cpp
SOURCES += ../../../../../vcglib/wrap/gui/trackball.cpp
SOURCES += ../../../../../vcglib/wrap/gui/trackmode.cpp


# Compile glew
DEFINES += GLEW_STATIC
INCLUDEPATH += ../../../../../code/lib/glew/include
SOURCES += ../../../../../code/lib/glew/src/glew.c

# Awful problem with windows..
win32{
  DEFINES += NOMINMAX
}

# Input
HEADERS += mainwindow.h
HEADERS += glarea.h

SOURCES += main.cpp
SOURCES += mainwindow.cpp
SOURCES += glarea.cpp

FORMS += mainwindow.ui
