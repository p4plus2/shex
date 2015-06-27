#ifndef MENU_MANAGER_H
#define MENU_MANAGER_H

#include <QUndoGroup>

#include "menus/abstract_menu_item.h"
#include "events/event_types.h"

class QMenuBar;
class main_window;

class menu_manager : public QObject
{
		Q_OBJECT
	public:
		explicit menu_manager(main_window *parent, QMenuBar *m, QUndoGroup *u);
		~menu_manager();
		void connect_to_widget(QObject *object, event_types event){ event_map[event] = object; }
		void group_connect_to_widget(QObject *object, event_types event);
		
	public slots:
		void post_event(QEvent *event);
		
	private:
		typedef QKeySequence hotkey;
		
		QMenuBar *menu_bar;
		QList<QMenu *> menu_list;
		QMap<event_types, QObject *> event_map;
		static main_window *window;
		
		void create_menus();
		void create_actions(QUndoGroup *undo_group);
		QMenu *find_menu(QString id);
		void enable_checkable(QActionGroup *group);
		void check_enabled(QMenu *menu);
		
		template <typename T, typename S>
		void add_toggle_action(QString text, S type, toggle_function toggle, hotkey key, QMenu *menu);
		template <typename T, typename S>
		void add_action(QString text, S type, hotkey key, QMenu *menu);
		template <typename T, typename S>
		void add_check_action(QString text, S type, hotkey key, QMenu *menu);
		template <typename T, typename S>
		void add_group_action(QString text, S type, hotkey key, QMenu *menu, QActionGroup *group);
};

#endif // MENU_MANAGER_H
