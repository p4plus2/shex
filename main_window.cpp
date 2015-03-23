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
	connect(tab_widget, &QTabWidget::tabCloseRequested, this, &main_window::close_tab);
	connect(tab_widget, &QTabWidget::currentChanged, this, &main_window::changed_tab);
	menu_controller->connect_to_widget(this, WINDOW_EVENT);
	menu_controller->connect_to_widget(dialog_controller, DIALOG_EVENT);
#ifdef USE_DEFAULT_ROM
	create_new_tab("SMW.smc");
#endif
}

bool main_window::close_tab(int i)
{
	hex_editor *editor = get_editor(i);
	if(editor->can_save()){
		typedef QMessageBox message;
		QString name = editor->get_file_name();
		int button = message::warning(this, "Save", "Do you wish to save any unsaved changed to " + name, 
		                              message::Yes | message::No | message::Cancel, message::Yes);
		switch(button){
			case message::Yes:
				if(!save(i)){
					return false;
				}
				
			break;
			case message::Cancel:
				return false;
			break;
			default:
			break;
		}
	}
	QWidget *widget = tab_widget->widget(i);
	tab_widget->removeTab(i);
	delete widget;
	if(!tab_widget->count()){
		has_active_editors = false;
	}
	return true;
}

void main_window::changed_tab(int i)
{
	if(i == -1){
		dialog_controller->set_active_editor(nullptr);
		menu_controller->connect_to_widget(nullptr, EDITOR_EVENT);
		return;
	}
	
	hex_editor *editor = get_editor(i);

	editor->set_focus();
	dialog_controller->set_active_editor(editor);
	menu_controller->connect_to_widget(editor, EDITOR_EVENT);
	menu_controller->connect_to_widget(get_panel(i), PANEL_EVENT);
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

bool main_window::save(bool override_name, int target)
{
	hex_editor *editor = (target != -1 ) ? get_editor(target) : get_editor(tab_widget->currentIndex());
	QString name = "";
	if(editor->new_file() || override_name){
		name = QFileDialog::getSaveFileName(this, "Save", QDir::homePath(), 
	                                            "ROM files (*.smc *.sfc);;All files(*.*)");
	}
	if(name == ""){
		return false;
	}
	editor->save(name);
	return true;
}

bool main_window::event(QEvent *event)
{
	if(event->type() != (QEvent::Type)WINDOW_EVENT){
		return QWidget::event(event);
	}
	switch(((window_event *)event)->sub_type()){
		case NEW:
			new_file();
			return true;
	        case OPEN:
			open();
			return true;
	        case SAVE:
			save();
			return true;
	        case SAVE_AS:
			save(true);
			return true;
	        case CLOSE_TAB:
			close_tab(tab_widget->currentIndex());
			return true;
	        case CLOSE:
			close();
			return true;
	        case VERSION:
			display_version_dialog();
			return true;
		default:
			qDebug() << "Bad event" << ((editor_event *)event)->sub_type();
			return false;
	}
}

void main_window::closeEvent(QCloseEvent *event)
{
	while(tab_widget->count()){
		if(!close_tab(0)){
			event->setAccepted(false);
			return;
		}
	}
	QApplication::quit();
	QMainWindow::closeEvent(event);
}

void main_window::init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar, panel_manager *panel)
{
	connect(scrollbar, &dynamic_scrollbar::valueChanged, editor, &hex_editor::slider_update);
	connect(editor, &hex_editor::update_slider, scrollbar, &dynamic_scrollbar::setValue);
	connect(editor, &hex_editor::update_range, scrollbar, &dynamic_scrollbar::set_range);
	connect(editor, &hex_editor::toggle_scroll_mode, scrollbar, &dynamic_scrollbar::toggle_mode);
	connect(editor, &hex_editor::update_status_text, statusbar, &QLabel::setText);
	connect(editor, &hex_editor::save_state_changed, this, &main_window::file_save_state);
	
	dialog_controller->connect_to_editor(editor);
	panel->connect_to_editor(editor);
	menu_controller->connect_to_widget(editor, EDITOR_EVENT);
	menu_controller->connect_to_widget(panel, PANEL_EVENT);
}

void main_window::create_new_tab(QString name, bool new_file)
{
	QWidget *widget = new QWidget(this);
	hex_editor *editor = new hex_editor(widget, name, undo_group, new_file);
	if(editor->load_error() != ""){
		QMessageBox::critical(this, "Invalid ROM", editor->load_error(), QMessageBox::Ok);
		delete editor;
		delete widget;
		return;
	}
	dynamic_scrollbar *scrollbar = new dynamic_scrollbar(editor);
	panel_manager *panel_controller = new panel_manager(editor);
	init_connections(editor, scrollbar, panel_controller);
	
	QHBoxLayout *hex_layout = new QHBoxLayout(widget);
	hex_layout->addWidget(editor);
	hex_layout->addWidget(scrollbar);
	hex_layout->addLayout(panel_controller->get_layout());
	widget->setLayout(hex_layout);
	tab_widget->addTab(widget, QFileInfo(name).fileName());
	
	tab_widget->setCurrentWidget(widget);
	editor->set_focus();
	setMinimumSize(tab_widget->minimumSize());
	panel_controller->init_displays();
	has_active_editors = true;
}

hex_editor *main_window::get_editor(int i) const
{
	return dynamic_cast<hex_editor *>(tab_widget->widget(i)->layout()->itemAt(0)->widget());
}

panel_manager *main_window::get_panel(int i) const
{
	return dynamic_cast<panel_manager *>(tab_widget->widget(i)->layout()->itemAt(2)->widget());
}

main_window::~main_window()
{
	character_mapper::delete_active_map();
}

bool main_window::has_active_editors = false;
