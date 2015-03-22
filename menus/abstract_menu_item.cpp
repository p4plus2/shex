#include "abstract_menu_item.h"
#include "menu_manager.h"
#include "debug.h"

abstract_menu_item::abstract_menu_item(QString txt, menu_manager *p) : QAction(txt, p){}

abstract_menu_item::abstract_menu_item(QString txt, QEvent *e, toggle_function t, 
                                       QKeySequence h, menu_manager *p, bool c) : QAction(txt, p)
{
	if(c){
		setCheckable(true);
		setChecked(false);
	}
	event = e;
	toggle = t;
	setShortcut(h);
	connect(this, &abstract_menu_item::triggered, this, &abstract_menu_item::send_event);
}

void abstract_menu_item::send_event()
{
	((menu_manager *)parent())->post_event(event);
}

abstract_menu_item::~abstract_menu_item()
{
	delete event;
}
