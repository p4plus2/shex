#ifndef EDITOR_MENU_ITEM_H
#define EDITOR_MENU_ITEM_H

#include "menus/abstract_menu_item.h"
#include <QKeySequence>

class hex_editor;

class editor_menu_item : public abstract_menu_item
{
		Q_OBJECT
	public:
		using abstract_menu_item::abstract_menu_item;
		virtual void connect_to_widget(hex_editor *editor);
		
	signals:
		
	public slots:
		
};

#endif // EDITOR_MENU_ITEM_H
