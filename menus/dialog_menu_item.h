#ifndef DIALOG_MENU_ITEM_H
#define DIALOG_MENU_ITEM_H

#include "menus/abstract_menu_item.h"
class dialog_manager;

class dialog_menu_item : public abstract_menu_item
{
		Q_OBJECT
	public:
		using abstract_menu_item::abstract_menu_item;
		virtual void connect_to_widget(dialog_manager *dialog_controller);
	signals:
		
	public slots:
		
};

#endif // DIALOG_MENU_ITEM_H
