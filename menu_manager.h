#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <QObject>
#include <QList>
#include <QMenu>
#include <QActionGroup>
#include <QCoreApplication>
#include <QMap>
#include "menus/abstract_menu_item.h"
#include "events/event_types.h"

class QMenuBar;

class menu_manager : public QObject
{
		Q_OBJECT
	public:
		explicit menu_manager(QObject *parent, QMenuBar *m, QUndoGroup *u);
		~menu_manager();
		void connect_to_widget(QObject *object, event_types event){ event_map[event] = object; }
		
	public slots:
		void post_event(QEvent *event);
		
	private:
		typedef QKeySequence hotkey;
		
		QMenuBar *menu_bar;
		QList<QMenu *> menu_list;
		QMap<event_types, QObject *> event_map;
		
		void create_menus();
		void create_actions(QUndoGroup *undo_group);
		QMenu *find_menu(QString id);
		void enable_checkable(QActionGroup *group);
		void check_enabled(QMenu *menu);
		
		template <typename T, typename S>
		void add_toggle_action(QString text, S type, toggle_function toggle, hotkey key, QMenu *menu);
		template <typename T, typename S>
		void add_action(QString text, S type, hotkey key, QMenu *menu, bool check = false);
		template <typename T, typename S>
		void add_group_action(QString text, S type, hotkey key, QMenu *menu, QActionGroup *group);
};

#endif // MENU_MANAGER_H
