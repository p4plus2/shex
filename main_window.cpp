#include "main_window.h"

#include <QHBoxLayout>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	statusbar = statusBar();
	editor = new hex_editor(this);
	scrollbar = new dynamic_scrollbar(editor);
	
	init_connections();
	
	QWidget* widget = new QWidget(this);
	
	QHBoxLayout *hex_layout = new QHBoxLayout(widget);
	hex_layout->addWidget(editor);
	hex_layout->addWidget(scrollbar);
	
	widget->setLayout(hex_layout);
	setCentralWidget(widget);
}

void main_window::init_connections()
{
	connect(scrollbar, SIGNAL(valueChanged(int)), editor, SLOT(slider_update(int)));
	connect(editor, SIGNAL(update_slider(int)), scrollbar, SLOT(setValue(int)));
	connect(editor, SIGNAL(update_range(int)), scrollbar, SLOT(set_range(int)));
	connect(editor, SIGNAL(toggle_scroll_mode(bool)), scrollbar, SLOT(toggle_mode(bool)));
	connect(scrollbar, SIGNAL(auto_scroll_action(bool)), editor, SLOT(control_auto_scroll(bool)));
	connect(editor, SIGNAL(update_status_text(QString)), statusbar, SLOT(showMessage(QString)));
}

main_window::~main_window()
{
	
}
