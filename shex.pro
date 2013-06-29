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
    undo_commands.cpp \
    dialogs/select_range_dialog.cpp \
    dialogs/goto_dialog.cpp \
    rom_metadata.cpp \
    dialogs/metadata_editor_dialog.cpp \
    dialog_manager.cpp \
    dialogs/expand_rom_dialog.cpp \
    dialogs/abstract_dialog.cpp \
    menu_manager.cpp \
    menus/abstract_menu_item.cpp \
    menus/history_menu_item.cpp \
    dialogs/find_replace_dialog.cpp \
    character_mapper.cpp

HEADERS  += main_window.h \
    hex_editor.h \
    dynamic_scrollbar.h \
    version.h \
    rom_buffer.h \
    undo_commands.h \
    dialogs/select_range_dialog.h \
    dialogs/goto_dialog.h \
    rom_metadata.h \
    debug.h \
    dialogs/metadata_editor_dialog.h \
    dialog_manager.h \
    dialogs/expand_rom_dialog.h \
    dialogs/abstract_dialog.h \
    menu_manager.h \
    menus/abstract_menu_item.h \
    menus/history_menu_item.h \
    menus/generic_menu_item.h \
    dialogs/find_replace_dialog.h \
    character_mapper.h

OTHER_FILES += \
    version.sh


