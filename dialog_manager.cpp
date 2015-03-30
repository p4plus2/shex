#include "dialog_manager.h"
#include "dialogs/find_replace_dialog.h"
#include "dialogs/goto_dialog.h"
#include "dialogs/select_range_dialog.h"
#include "dialogs/expand_rom_dialog.h"
#include "dialogs/metadata_editor_dialog.h"
#include "dialogs/map_editor_dialog.h"
#include "dialogs/settings_dialog.h"
#include "hex_editor.h"

dialog_manager::dialog_manager(QWidget *parent) :
        QObject(parent)
{
	dialog_map[GOTO] = new goto_dialog(parent);
	dialog_map[SELECT_RANGE] = new select_range_dialog(parent);
	dialog_map[EXPAND] = new expand_ROM_dialog(parent);
	dialog_map[METADATA_EDITOR] = new metadata_editor_dialog(parent);
	dialog_map[FIND_REPLACE] = new find_replace_dialog(parent);
	dialog_map[MAP_EDITOR] = new map_editor_dialog(parent);
	dialog_map[SETTINGS] = new settings_dialog(parent);
}

void dialog_manager::connect_to_editor(hex_editor *editor)
{
#define CONNECT(D,E,S,T) connect((D *)find_dialog(E), &D::S, editor, &hex_editor::T);
	CONNECT(goto_dialog, GOTO, triggered, goto_offset);
	CONNECT(select_range_dialog, SELECT_RANGE, triggered, select_range);
	CONNECT(find_replace_dialog, FIND_REPLACE, count, count);
	CONNECT(find_replace_dialog, FIND_REPLACE, search, search);
	CONNECT(find_replace_dialog, FIND_REPLACE, replace, replace);
	CONNECT(find_replace_dialog, FIND_REPLACE, replace_all, replace_all);
#undef CONNECT
}

void dialog_manager::set_active_editor(hex_editor *editor)
{
	for(auto &dialog : dialog_map){
		dialog->set_active_editor(editor);
		dialog->refresh();
	}
}

void dialog_manager::raise_dialog(dialog_events id)
{
	abstract_dialog *dialog = find_dialog(id);
	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

bool dialog_manager::event(QEvent *event)
{
	if(event->type() != (QEvent::Type)DIALOG_EVENT){
		return QObject::event(event);
	}
	raise_dialog(((dialog_event *)event)->sub_type());
	return true;
}

abstract_dialog *dialog_manager::find_dialog(dialog_events id)
{
	if(dialog_map.contains(id)){
		return dialog_map[id];
	}
	qDebug() << "Error: Dialog " << id << " not found";
	return 0;
}
