#ifndef HISTORY_MENU_ITEM_H
#define HISTORY_MENU_ITEM_H
#include "menus/abstract_menu_item.h"

class QUndoGroup;
class main_window;

class history_menu_item : public abstract_menu_item
{
		Q_OBJECT
	public:
		history_menu_item(QString text, QString r, QString t, QKeySequence hotkey, QWidget *parent);
		virtual void connect_to_widget(QUndoGroup *group);
		virtual void connect_to_widget(main_window *window);
		
	public slots:
		void set_prefix(const QString &t);
		
	private:
		QString prefix;
		
};

#endif // HISTORY_MENU_ITEM_H
