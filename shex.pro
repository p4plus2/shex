#-------------------------------------------------
#
# Project created by QtCreator 2011-12-02T22:12:14
#
#-------------------------------------------------

QT       += core gui

TARGET = shex
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++0x

SOURCES += main.cpp\
        main_window.cpp \
    hex_editor.cpp \
    dynamic_scrollbar.cpp \
    rom_buffer.cpp \
    undo_commands.cpp

HEADERS  += main_window.h \
    hex_editor.h \
    dynamic_scrollbar.h \
    version.h \
    rom_buffer.h \
    undo_commands.h

OTHER_FILES += \
    version.sh


