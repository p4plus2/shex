#include "dialog_manager.h"
#include "hex_editor.h"

dialog_manager::dialog_manager(QObject *parent) :
        QObject(parent)
{
}

void dialog_manager::connect_to_editor(hex_editor *editor)
{
	connect(goto_window, SIGNAL(triggered(int,bool)), editor, SLOT(goto_offset(int, bool)));
	connect(select_range_window, SIGNAL(triggered(int,int,bool)), editor, SLOT(select_range(int,int,bool)));
}

void dialog_manager::raise_dialog(QDialog *dialog)
{
	dialog->show();
	dialog->raise();
	dialog->activateWindow();
}

dialog_manager::~dialog_manager()
{
	delete goto_window;
	delete select_range_window;
	delete expand_ROM_window;
	delete metadata_editor_window;
}
