#include "main_window.h"
#include "hex_editor.h"
#include "dynamic_scrollbar.h"

#include <QStatusBar>
#include <QHBoxLayout>
#include <QTabWidget>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	statusbar = statusBar();
	
	QWidget* widget = new QWidget(this);
	tab_widget = new QTabWidget(this);
	QHBoxLayout *tab_layout = new QHBoxLayout(widget);
	tab_layout->addWidget(tab_widget);
	widget->setLayout(tab_layout);
	setCentralWidget(widget);
	tab_widget->setTabsClosable(true);
	tab_widget->setMovable(true);
	setMinimumSize(600, 700);
	connect(tab_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(close_tab(int)));
	
	create_new_tab("testing.smc");
	create_new_tab("something.smc");
}

void main_window::close_tab(int i)
{
	QWidget *widget = tab_widget->widget(i);
	tab_widget->removeTab(i);
	delete widget;
}

void main_window::init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar)
{
	connect(scrollbar, SIGNAL(valueChanged(int)), editor, SLOT(slider_update(int)));
	connect(editor, SIGNAL(update_slider(int)), scrollbar, SLOT(setValue(int)));
	connect(editor, SIGNAL(update_range(int)), scrollbar, SLOT(set_range(int)));
	connect(editor, SIGNAL(toggle_scroll_mode(bool)), scrollbar, SLOT(toggle_mode(bool)));
	connect(scrollbar, SIGNAL(auto_scroll_action(bool)), editor, SLOT(control_auto_scroll(bool)));
	connect(editor, SIGNAL(update_status_text(QString)), statusbar, SLOT(showMessage(QString)));
}

void main_window::create_new_tab(QString name)
{
	hex_editor *editor = new hex_editor(this);
	dynamic_scrollbar *scrollbar = new dynamic_scrollbar(editor);
	init_connections(editor, scrollbar);
	
	QWidget *widget = new QWidget(this);
	QHBoxLayout *hex_layout = new QHBoxLayout(widget);
	hex_layout->addWidget(editor);
	hex_layout->addWidget(scrollbar);
	widget->setLayout(hex_layout);
	tab_widget->addTab(widget, name);
}

main_window::~main_window()
{
	
}
