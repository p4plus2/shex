#include "window_menu_item.h"
#include "main_window.h"

void window_menu_item::connect_to_widget(main_window *window)
{
	connect(this, SIGNAL(triggered()),window, run.toAscii().data());
	if(toggle != ""){
		connect(window, toggle.toAscii().data(), this, SLOT(setEnabled(bool)));
	}
}
