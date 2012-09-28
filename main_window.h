#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QStatusBar>
#include "hex_editor.h"
#include "dynamic_scrollbar.h"

class main_window : public QMainWindow
{
		Q_OBJECT
		
	public:
		main_window(QWidget *parent = 0);
		~main_window();
		
	private:
		hex_editor *editor;
		dynamic_scrollbar *scrollbar;
		QStatusBar *statusbar;
		
		void init_connections();
		
};

#endif // MAIN_WINDOW_H
