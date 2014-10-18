#include "abstract_dialog.h"
#include "debug.h"

abstract_dialog::abstract_dialog(QWidget *parent) : QDialog(parent)
{
	setWindowFlags(Qt::Tool);
}

void abstract_dialog::set_active_editor(hex_editor *editor)
{
	active_editor = editor;
}

void abstract_dialog::refresh()
{
	if(!active_editor){
		close();
	}
}
