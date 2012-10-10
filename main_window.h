#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class hex_editor;
class dynamic_scrollbar;

class main_window : public QMainWindow
{
		Q_OBJECT
		
	public:
		main_window(QWidget *parent = 0);
		~main_window();
		
	public slots:
		void close_tab(int i);
		
	private:
		QStatusBar *statusbar;
		QTabWidget *tab_widget;
		
		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar);
		void create_new_tab(QString name);
		
};

#endif // MAIN_WINDOW_H
