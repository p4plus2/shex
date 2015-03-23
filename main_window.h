#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QUndoGroup>
#include <QAction>
#include "dialog_manager.h"
#include "menu_manager.h"
#include "panel_manager.h"
#include "rom_buffer.h"
#include <QLayout>

#include "debug.h"
class hex_editor;
class dynamic_scrollbar;
class QLabel;
class disassembler;
class bookmarks;

class main_window : public QMainWindow
{
		Q_OBJECT
		
	public:
		main_window(QWidget *parent = 0);
		~main_window();
		
	public slots:
		bool close_tab(int i);
		void changed_tab(int i);
		void file_save_state(bool clean);
		void new_file();
		void open();
		bool save(bool override_name = false, int target = -1);
		static inline bool active_editors(){ return has_active_editors; }
		
	protected:
		virtual bool event(QEvent *event);
		virtual void closeEvent(QCloseEvent *event);

	private:
		QLabel *statusbar = new QLabel(this);
		QTabWidget *tab_widget = new QTabWidget(this);
		QUndoGroup *undo_group = new QUndoGroup(this);
		dialog_manager *dialog_controller = new dialog_manager(this);
		menu_manager *menu_controller = new menu_manager(this, menuBar(), undo_group);
		int new_counter = 0;
		
		static bool has_active_editors;

		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar, panel_manager *panel);
		void create_new_tab(QString name, bool new_file = false);
		hex_editor *get_editor(int i) const;
		panel_manager *get_panel(int i) const;
		
};

#endif // MAIN_WINDOW_H
