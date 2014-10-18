#include "dialog_manager.h"
#include "hex_editor.h"

dialog_manager::dialog_manager(QWidget *parent) :
        QObject(parent)
{
	dialog_list.append(new goto_dialog(parent));
	dialog_list.append(new select_range_dialog(parent));
	dialog_list.append(new expand_ROM_dialog(parent));
	dialog_list.append(new metadata_editor_dialog(parent));
	dialog_list.append(new find_replace_dialog(parent));
	dialog_list.append(new map_editor_dialog(parent));
}

void dialog_manager::connect_to_editor(hex_editor *editor)
{
#define GET_DIALOG(D) (D##_dialog *)find_dialog(#D)
	connect(GET_DIALOG(goto), &goto_dialog::triggered, editor, &hex_editor::goto_offset);
	connect(GET_DIALOG(select_range), &select_range_dialog::triggered, editor, &hex_editor::select_range);
	connect(GET_DIALOG(find_replace), &find_replace_dialog::count, editor, &hex_editor::count);
	connect(GET_DIALOG(find_replace), &find_replace_dialog::search,editor, &hex_editor::search);
	connect(GET_DIALOG(find_replace), &find_replace_dialog::replace, editor, &hex_editor::replace);
	connect(GET_DIALOG(find_replace), &find_replace_dialog::replace_all, editor, &hex_editor::replace_all);
#undef GET_DIALOG
}

void dialog_manager::set_active_editor(hex_editor *editor)
{
	foreach(abstract_dialog *dialog, dialog_list){
		dialog->set_active_editor(editor);
		dialog->refresh();
	}
}

void dialog_manager::raise_dialog(QString id)
{
	abstract_dialog *dialog = find_dialog(id);
	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

abstract_dialog *dialog_manager::find_dialog(QString id)
{
	foreach(abstract_dialog *dialog, dialog_list){
		if(dialog->id() == id){
			return dialog;
		}
	}
	qDebug() << "Error: Dialog " << id << " not found";
	return 0;
}
