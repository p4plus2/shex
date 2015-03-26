#ifndef HISTORY_MENU_ITEM_H
#define HISTORY_MENU_ITEM_H
#include "menus/abstract_menu_item.h"

class QUndoGroup;
class hex_editor;

class history_menu_item : public abstract_menu_item
{
		Q_OBJECT
	public:
		history_menu_item(QString t, QEvent *e, QKeySequence hotkey, menu_manager *parent, QUndoGroup *g);
		
	public slots:
		void set_prefix(const QString &t);
		
	private:
		QString prefix;
		
};

#endif // HISTORY_MENU_ITEM_H
