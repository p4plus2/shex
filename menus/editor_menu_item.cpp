#include "editor_menu_item.h"
#include "hex_editor.h"
#include "debug.h"

void editor_menu_item::connect_to_widget(hex_editor *editor)
{
	connect(this, SIGNAL(triggered()),editor, run.toAscii().data());
	if(toggle != ""){
		connect(editor, toggle.toAscii().data(), this, SLOT(setEnabled(bool)));
	}
}
