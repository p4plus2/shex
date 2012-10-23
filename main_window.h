#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QUndoGroup>

#include "dialogs/goto_dialog.h"
#include "dialogs/select_range_dialog.h"

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
		
		inline void show_goto_dialog() { raise_dialog(goto_window); }
		inline void show_select_range_dialog() { raise_dialog(select_range_window); }
		
	private:
		QLabel *statusbar;
		QTabWidget *tab_widget;
		QUndoGroup *undo_group;
		int new_counter;
		
		goto_dialog *goto_window;
		select_range_dialog *select_range_window;
		
		QMenu *file_menu;
		QMenu *edit_menu;
		QMenu *navigation_menu;
		QMenu *options_menu;
		QMenu *help_menu;
		
		QAction *new_file_action;
		QAction *open_action;
		QAction *save_action;
		QAction *exit_action;
		QAction *delete_action;
		QAction *select_all_action;
		
		QAction *goto_action;
		QAction *select_range_action;
		
		QAction *undo_action;
		QAction *redo_action;
		QAction *cut_action;
		QAction *copy_action;
		QAction *paste_action;
		
		QAction *scrollbar_toggle_action;
		
		QAction *version_action;
		
		void create_menu();
		void create_actions();
		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar);
		void create_new_tab(QString name, bool new_file = false);
		hex_editor *get_editor(int i);
		void raise_dialog(QDialog *dialog);
		
};

#endif // MAIN_WINDOW_H
