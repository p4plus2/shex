#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "dialogs/goto_dialog.h"
#include "dialogs/select_range_dialog.h"

#include <QMainWindow>
#include <QUndoGroup>
#include <QAction>

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
		QLabel *statusbar = new QLabel(this);
		QTabWidget *tab_widget = new QTabWidget(this);
		QUndoGroup *undo_group = new QUndoGroup(this);
		int new_counter = 0;
		
		goto_dialog *goto_window = new goto_dialog(this);
		select_range_dialog *select_range_window = new select_range_dialog(this);
		
		QMenu *file_menu;
		QMenu *edit_menu;
		QMenu *navigation_menu;
		QMenu *options_menu;
		QMenu *help_menu;
		
		QAction *new_file_action = new QAction("&New", this);
		QAction *open_action = new QAction("&Open...", this);
		QAction *save_action = new QAction("&Save", this);
		QAction *exit_action = new QAction("E&xit", this);
		
		QAction *undo_action = undo_group->createUndoAction(this);
		QAction *redo_action = undo_group->createRedoAction(this);
		QAction *cut_action = new QAction("Cu&t", this);
		QAction *copy_action = new QAction("&Copy", this);
		QAction *paste_action = new QAction("&Paste", this);
		QAction *delete_action = new QAction("&Delete", this);
		QAction *select_all_action = new QAction("&Select all", this);
		QAction *select_range_action = new QAction("&Select Range", this);
		
		QAction *goto_action = new QAction("&Goto offset", this);
		
		QAction *scrollbar_toggle_action = new QAction("&Scrollbar toggle", this);
		
		QAction *version_action = new QAction("&Version", this);
		
		void create_menu();
		void init_actions();
		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar);
		void create_new_tab(QString name, bool new_file = false);
		hex_editor *get_editor(int i);
		void raise_dialog(QDialog *dialog);
		
};

#endif // MAIN_WINDOW_H
