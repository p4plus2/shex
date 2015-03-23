#ifndef EVENT_TYPES
#define EVENT_TYPES
#include "general_event.h"
#include "rom_buffer.h"

enum editor_events{
	CUT = QEvent::User+1,
	COPY,
	PASTE,
	DELETE_TEXT,
	SELECT_ALL,
	BRANCH,
	JUMP,
	DISASSEMBLE,
	SCROLL_MODE,
	NO_SPACES, SPACES, HEX_FORMAT,
	ASM_BYTE_TABLE, ASM_WORD_TABLE, ASM_LONG_TABLE, 
	C_SOURCE,
	UNDO,
	REDO,
	EDITOR_EVENT_MAX
};

enum window_events{
	NEW = EDITOR_EVENT_MAX+1,
        OPEN,
        SAVE,
        SAVE_AS,
        CLOSE_TAB,
        CLOSE,
        VERSION,
	WINDOW_EVENT_MAX
};

enum dialog_events{
	SELECT_RANGE = WINDOW_EVENT_MAX+1,
        FIND_REPLACE,
        GOTO,
        EXPAND,
        METADATA_EDITOR,
        MAP_EDITOR,
	DIALOG_EVENT_MAX
};

enum panel_events{
	DISASSEMBLER = DIALOG_EVENT_MAX+1,
	BOOKMARKS,
	PANEL_EVENT_MAX
};

enum event_types : int{
	EDITOR_EVENT = EDITOR_EVENT_MAX,
	WINDOW_EVENT = WINDOW_EVENT_MAX,
	DIALOG_EVENT = DIALOG_EVENT_MAX,
	PANEL_EVENT = PANEL_EVENT_MAX,
	UNIMPLEMENTED
};

typedef general_event<editor_events, EDITOR_EVENT> editor_event;
typedef general_event<window_events, WINDOW_EVENT> window_event;
typedef general_event<dialog_events, DIALOG_EVENT> dialog_event;
typedef general_event<panel_events, PANEL_EVENT> panel_event;

#endif // EVENT_TYPES

