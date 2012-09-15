#include "main_window.h"
#include "hex_editor.h"

#include <QHBoxLayout>
#include <QScrollBar>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	hex_editor *editor = new hex_editor(this);
	QScrollBar *scrollbar = new QScrollBar(this);
	
	connect(scrollbar, SIGNAL(valueChanged(int)), editor, SLOT(slider_update(int)));
	connect(editor, SIGNAL(move_slider(int)), scrollbar, SLOT(setValue(int)));
	
	scrollbar->setRange(0,editor->get_max_lines());
	
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
