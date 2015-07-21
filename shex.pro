#-------------------------------------------------
#
# Project created by QtCreator 2011-12-02T22:12:14
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = shex
TEMPLATE = app

QMAKE_CXXFLAGS += -std=c++14 -Wextra 
#-fsanitize=memory -fno-omit-frame-pointer -fsanitize-blacklist=blacklist.txt
#QMAKE_LFLAGS += -fsanitize=memory

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
    character_mapper.cpp \
    dialogs/map_editor_dialog.cpp \
    disassembly_cores/isa_65c816.cpp \
    disassembly_cores/disassembler_core.cpp \
    displays/hex_display.cpp \
    displays/text_display.cpp \
    displays/ascii_display.cpp \
    displays/address_display.cpp \
    selection.cpp \
    disassembly_cores/isa_spc700.cpp \
    panels/abstract_panel.cpp \
    panel_manager.cpp \
    panels/bookmark_panel.cpp \
    panels/disassembler_panel.cpp \
    object_group.cpp \
    settings_manager.cpp \
    dialogs/settings_dialog.cpp \
    editor_font.cpp \
    disassembly_cores/isa_gsu.cpp \
    dialogs/how_to_use_dialog.cpp

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
    dialogs/find_replace_dialog.h \
    character_mapper.h \
    dialogs/map_editor_dialog.h \
    disassembly_cores/isa_65c816.h \
    disassembly_cores/disassembler_core.h \
    displays/hex_display.h \
    displays/text_display.h \
    displays/ascii_display.h \
    displays/address_display.h \
    selection.h \
    disassembly_cores/isa_spc700.h \
    utility.h \
    events/event_types.h \
    events/general_event.h \
    panels/abstract_panel.h \
    panel_manager.h \
    panels/disassembler_panel.h \
    panels/bookmark_panel.h \
    object_group.h \
    settings_manager.h \
    events/settings_event.h \
    dialogs/settings_dialog.h \
    editor_font.h \
    disassembly_cores/isa_gsu.h \
    dialogs/how_to_use_dialog.h

OTHER_FILES += \
    version.sh

DISTFILES += \
    shex.supp
