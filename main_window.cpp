#include "main_window.h"
#include "hex_editor.h"
#include "dynamic_scrollbar.h"
#include "version.h"
#include "debug.h"
#include "character_mapper.h"

#include <QStatusBar>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QMenuBar>
#include <QFileDialog>
#include <QDesktopWidget>
#include <QLabel>

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	statusBar()->addWidget(statusbar);
	
	QWidget* widget = new QWidget(this);
	QHBoxLayout *tab_layout = new QHBoxLayout(widget);
	tab_layout->addWidget(tab_widget);
	widget->setLayout(tab_layout);
	setCentralWidget(widget);
	tab_widget->setTabsClosable(true);
	tab_widget->setMovable(true);
	setMinimumSize(600, QApplication::desktop()->height() < 650 ? 330 : 660);
	connect(tab_widget, SIGNAL(tabCloseRequested(int)), this, SLOT(close_tab(int)));
	connect(tab_widget, SIGNAL(currentChanged(int)), this, SLOT(changed_tab(int)));
	connect(this, SIGNAL(active_editors(bool)), dialog_controller, SIGNAL(active_editors(bool)));
	menu_controller->connect_to_widget(this);
	menu_controller->connect_to_widget(dialog_controller);
	menu_controller->connect_to_widget(undo_group);
#ifdef USE_DEFAULT_ROM
	create_new_tab("SMW.smc");
#endif
}

void main_window::close_tab(int i)
{
	hex_editor *editor = get_editor(i);
	if(editor->can_save()){
		typedef QMessageBox message;
		QString name = editor->get_file_name();
		int button = message::warning(this, "Save", "Do you wish to save any unsaved changed to " + name, 
		                              message::Yes | message::No | message::Cancel, message::Yes);
		switch(button){
			case message::Yes:
				save(i);
				
			break;
			case message::Cancel:
				return;
			break;
			default:
			break;
		}
	}
	QWidget *widget = tab_widget->widget(i);
	tab_widget->removeTab(i);
	delete widget;
	if(!tab_widget->count()){
		emit active_editors(false);
	}
}

void main_window::changed_tab(int i)
{
	if(i == -1){
		dialog_controller->set_active_editor(nullptr);
		return;
	}
	
	hex_editor *editor = get_editor(i);

	editor->set_focus();
	dialog_controller->set_active_editor(editor);
}

void main_window::file_save_state(bool clean)
{
	hex_editor *editor = get_editor(tab_widget->currentIndex());
	if(clean){
		tab_widget->setTabText(tab_widget->currentIndex(), editor->get_file_name());
	}else{
		tab_widget->setTabText(tab_widget->currentIndex(), "* "+editor->get_file_name());
	}
}

void main_window::new_file()
{
	new_counter++;
	create_new_tab("Untitled_"+QString::number(new_counter), true);
}

void main_window::open()
{
	QStringList file_list = QFileDialog::getOpenFileNames(this, "Open file(s)", QDir::homePath(),
	                                                      "ROM files (*.smc *.sfc);;All files(*.*)");
	QString file_name;
	foreach(file_name, file_list){
		create_new_tab(file_name);
	}
}

void main_window::save(bool override_name, int target)
{
	hex_editor *editor = (target != -1 ) ? get_editor(target) : get_editor(tab_widget->currentIndex());
	QString name = "";
	if(editor->new_file() || override_name){
		name = QFileDialog::getSaveFileName(this, "Save", QDir::homePath(), 
	                                            "ROM files (*.smc *.sfc);;All files(*.*)");
	}
	editor->save(name);
}

void main_window::version()
{
	display_version_dialog();
}

void main_window::closeEvent(QCloseEvent *event)
{
	QApplication::quit();
	QMainWindow::closeEvent(event);
}

void main_window::init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar)
{
	connect(scrollbar, SIGNAL(valueChanged(int)), editor, SLOT(slider_update(int)));
	connect(editor, SIGNAL(update_slider(int)), scrollbar, SLOT(setValue(int)));
	connect(editor, SIGNAL(update_range(int)), scrollbar, SLOT(set_range(int)));
	connect(editor, SIGNAL(toggle_scroll_mode(bool)), scrollbar, SLOT(toggle_mode(bool)));
	connect(scrollbar, SIGNAL(auto_scroll_action(bool)), editor, SLOT(control_auto_scroll(bool)));
	connect(editor, SIGNAL(update_status_text(QString)), statusbar, SLOT(setText(QString)));
	connect(editor, SIGNAL(can_save(bool)), this, SLOT(file_save_state(bool)));
	
	dialog_controller->connect_to_editor(editor);
	menu_controller->connect_to_widget(editor);
}

void main_window::create_new_tab(QString name, bool new_file)
{
	QWidget *widget = new QWidget(this);
	hex_editor *editor = new hex_editor(widget, name, undo_group, new_file);
	dynamic_scrollbar *scrollbar = new dynamic_scrollbar(editor);
	init_connections(editor, scrollbar);
	
	QHBoxLayout *hex_layout = new QHBoxLayout(widget);
	hex_layout->addWidget(editor);
	hex_layout->addWidget(scrollbar);
	widget->setLayout(hex_layout);
	tab_widget->addTab(widget, QFileInfo(name).fileName());
	
	tab_widget->setCurrentWidget(widget);
	editor->set_focus();
	setMinimumSize(tab_widget->minimumSize());
	emit active_editors(true);
}

hex_editor *main_window::get_editor(int i)
{
	return dynamic_cast<hex_editor *>(tab_widget->widget(i)->layout()->itemAt(0)->widget());
}

main_window::~main_window()
{
	character_mapper::delete_active_map();
}
