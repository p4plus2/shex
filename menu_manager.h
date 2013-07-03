#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <QObject>
#include <QList>
#include <QMenu>
#include "menus/abstract_menu_item.h"

class QMenuBar;
class hex_editor;
class QMenu;
class main_window;

class menu_manager : public QObject
{
		Q_OBJECT
	public:
		explicit menu_manager(QObject *parent, QMenuBar *m);
		~menu_manager();
		template <typename W> void connect_to_widget(W widget)
		{
			foreach(QMenu *menu, menu_list){
				foreach(QAction *action, menu->actions()){
					if(!action->isSeparator()){
						dynamic_cast<abstract_menu_item *>(action)->connect_to_widget(widget);
					}
				}
			}
		}
		
	private:
		QMenuBar *menu_bar;
		QList<QMenu *> menu_list;
		void create_menus();
		void create_actions();
		QMenu *find_menu(QString id);
		
};

#endif // MENU_MANAGER_H
