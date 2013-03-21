#include "abstract_dialog.h"

#include "hex_editor.h"

abstract_dialog::abstract_dialog()
{
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
