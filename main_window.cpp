#include "main_window.h"
#include "hex_editor.h"
#include "dynamic_scrollbar.h"

#include <QStatusBar>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QMenuBar>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	statusbar = statusBar();
	create_menu();
	
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
	connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(changed_tab(int)));
	
	create_new_tab("testing.smc");
	create_new_tab("something.smc");
}

void main_window::close_tab(int i)
{
	QWidget *widget = tab_widget->widget(i);
	tab_widget->removeTab(i);
	delete widget;
}

void main_window::changed_tab(int i)
{
	undo_action->disconnect();
	redo_action->disconnect();
	cut_action->disconnect();
	copy_action->disconnect();
	paste_action->disconnect();
	
	hex_editor *editor = dynamic_cast<hex_editor *>(tab_widget->widget(i)->layout()->itemAt(0)->widget());
	connect(undo_action, SIGNAL(triggered()), editor, SLOT(undo()));
	connect(redo_action, SIGNAL(triggered()), editor, SLOT(redo()));
	connect(cut_action, SIGNAL(triggered()), editor, SLOT(cut()));
	connect(copy_action, SIGNAL(triggered()), editor, SLOT(copy()));
	connect(paste_action, SIGNAL(triggered()), editor, SLOT(paste()));
}

void main_window::new_file()
{
	qDebug() << ("Invoked <b>File|New</b>");
}

void main_window::open()
{
	qDebug() << ("Invoked <b>File|Open</b>");
}

void main_window::save()
{
	qDebug() << ("Invoked <b>File|Save</b>");
}

void main_window::create_menu()
{
	create_actions();
	file_menu = menuBar()->addMenu("&File");
	file_menu->addAction(new_file_action);
	file_menu->addAction(open_action);
	file_menu->addAction(save_action);
	file_menu->addSeparator();
	file_menu->addAction(exit_action);
	
	edit_menu = menuBar()->addMenu("&Edit");
	edit_menu->addAction(undo_action);
	edit_menu->addAction(redo_action);
	edit_menu->addSeparator();
	edit_menu->addAction(cut_action);
	edit_menu->addAction(copy_action);
	edit_menu->addAction(paste_action);
	edit_menu->addSeparator();
}

void main_window::create_actions()
{
	new_file_action = new QAction("&New", this);
	new_file_action->setShortcuts(QKeySequence::New);
	connect(new_file_action, SIGNAL(triggered()), this, SLOT(new_file()));
	
	open_action = new QAction("&Open...", this);
	open_action->setShortcuts(QKeySequence::Open);
	connect(open_action, SIGNAL(triggered()), this, SLOT(open()));
	
	save_action = new QAction("&Save", this);
	save_action->setShortcuts(QKeySequence::Save);
	connect(save_action, SIGNAL(triggered()), this, SLOT(save()));
	
	exit_action = new QAction("E&xit", this);
	exit_action->setShortcuts(QKeySequence::Quit);
	connect(exit_action, SIGNAL(triggered()), this, SLOT(close()));
	
	undo_action = new QAction("&Undo", this);
	undo_action->setShortcuts(QKeySequence::Undo);
	
	redo_action = new QAction("&Redo", this);
	redo_action->setShortcuts(QKeySequence::Redo);
	
	cut_action = new QAction("Cu&t", this);
	cut_action->setShortcuts(QKeySequence::Cut);
	
	copy_action = new QAction("&Copy", this);
	copy_action->setShortcuts(QKeySequence::Copy);
	
	paste_action = new QAction("&Paste", this);
	paste_action->setShortcuts(QKeySequence::Paste);
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
