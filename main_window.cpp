#include "main_window.h"
#include "hex_editor.h"
#include "dynamic_scrollbar.h"

#include <QHBoxLayout>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	hex_editor *editor = new hex_editor(this);
	dynamic_scrollbar *scrollbar = new dynamic_scrollbar(editor);
	
	connect(scrollbar, SIGNAL(valueChanged(int)), editor, SLOT(slider_update(int)));
	connect(editor, SIGNAL(update_slider(int)), scrollbar, SLOT(setValue(int)));
	connect(editor, SIGNAL(update_range(int)), scrollbar, SLOT(set_range(int)));
	connect(editor, SIGNAL(toggle_scroll_mode(bool)), scrollbar, SLOT(toggle_mode(bool)));
	
	QWidget* widget = new QWidget(this);
	
	QHBoxLayout *hex_layout = new QHBoxLayout(widget);
	hex_layout->addWidget(editor);
	hex_layout->addWidget(scrollbar);
	
	widget->setLayout(hex_layout);
	setCentralWidget(widget);
}

main_window::~main_window()
{
	
}
