#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QUndoGroup>
#include <QAction>
#include "dialog_manager.h"
#include "menu_manager.h"

class hex_editor;
class dynamic_scrollbar;
class QLabel;

class main_window : public QMainWindow
{
		Q_OBJECT
		
	public:
		main_window(QWidget *parent = 0);
		~main_window();
		
	public slots:
		void close_tab(int i);
		void changed_tab(int i);
		void file_save_state(bool clean);
		void new_file();
		void open();
		void save(bool override_name = false, int target = -1);
		void version();
		
	signals:
		void active_editors(bool active);
		
	protected:
		virtual void closeEvent(QCloseEvent *event);
		
	private:
		QLabel *statusbar = new QLabel(this);
		QTabWidget *tab_widget = new QTabWidget(this);
		QUndoGroup *undo_group = new QUndoGroup(this);
		dialog_manager *dialog_controller = new dialog_manager(this);
		menu_manager *menu_controller = new menu_manager(this, menuBar());
		int new_counter = 0;

		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar);
		void create_new_tab(QString name, bool new_file = false);
		hex_editor *get_editor(int i);
		
};

#endif // MAIN_WINDOW_H
