#include "dialog_menu_item.h"
#include "dialog_manager.h"

void dialog_menu_item::connect_to_widget(dialog_manager *dialog_controller)
{
	connect(this, SIGNAL(triggered()), dialog_controller, run.toAscii().data());
	if(toggle != ""){
		connect(dialog_controller, toggle.toAscii().data(), this, SLOT(setEnabled(bool)));
	}
}
