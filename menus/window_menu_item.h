#ifndef WINDOW_MENU_ITEM_H
#define WINDOW_MENU_ITEM_H
#include "abstract_menu_item.h"

class window_menu_item : public abstract_menu_item
{
		Q_OBJECT
	public:
		using abstract_menu_item::abstract_menu_item;
		virtual void connect_to_widget(main_window *window);
	signals:
		
	public slots:
		
};

#endif // WINDOW_MENU_ITEM_H
