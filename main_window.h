#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QUndoGroup>

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
		void save();
		void version();
		void update_hex_editor();
		
	private:
		QLabel *statusbar;
		QTabWidget *tab_widget;
		QUndoGroup *undo_group;
		int new_counter;
		
		
		QMenu *file_menu;
		QMenu *edit_menu;
		QMenu *options_menu;
		QMenu *help_menu;
		
		QAction *new_file_action;
		QAction *open_action;
		QAction *save_action;
		QAction *exit_action;
		
		QAction *undo_action;
		QAction *redo_action;
		QAction *cut_action;
		QAction *copy_action;
		QAction *paste_action;
		
		QAction *version_action;
		
		void create_menu();
		void create_actions();
		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar);
		void create_new_tab(QString name, bool new_file = false);
		hex_editor *get_editor(int i);
		
};

#endif // MAIN_WINDOW_H
