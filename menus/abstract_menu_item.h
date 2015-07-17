#ifndef ABSTRACT_MENU_ITEM_H
#define ABSTRACT_MENU_ITEM_H

#include <QAction>

#include "debug.h"

class menu_manager;
class QEvent;

typedef bool(*toggle_function)();

class abstract_menu_item : public QAction
{
		Q_OBJECT
	public:
		abstract_menu_item(QString txt, menu_manager *p);
		abstract_menu_item(QString txt, QEvent *e, toggle_function t, QKeySequence h, menu_manager *p, bool c);
		~abstract_menu_item();
		
		void check_enabled(){ if(toggle){ setEnabled(toggle()); } }
		
	protected:
		QEvent *event;
		toggle_function toggle = nullptr;
		
	private slots:
		void send_event();
		
};

#endif // ABSTRACT_MENU_ITEM_H
