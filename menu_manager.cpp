#include <QMenuBar>

#include "menu_manager.h"
#include "menus/history_menu_item.h"
#include "hex_editor.h"
#include "main_window.h"
#include "object_group.h"
#include "debug.h"

menu_manager::menu_manager(QObject *parent, QMenuBar *m, QUndoGroup *u) :
        QObject(parent)
{
	menu_bar = m;
	create_menus();
	create_actions(u);
}

void menu_manager::group_connect_to_widget(QObject *object, event_types event)
{
	if(!event_map.contains(event)){
		event_map[event] = new object_group(this);
	}
	((object_group *)event_map[event])->add_to_group(object);
}

void menu_manager::create_menus()
{
	menu_list.append(new QMenu("&File"));
	menu_list.append(new QMenu("&Edit"));
	menu_list.append(new QMenu("&Navigation"));
	menu_list.append(new QMenu("&ROM utilities"));
	menu_list.append(new QMenu("&Options"));
	menu_list.append(new QMenu("&Help"));
	
	for(auto &menu : menu_list){
		connect(menu, &QMenu::aboutToShow, this, [=](){ check_enabled(menu); });
		menu_bar->addMenu(menu);
	}
	menu_list.append(new QMenu("&Copy style"));
}

template <typename T, typename S>
void menu_manager::add_toggle_action(QString text, S type, toggle_function toggle, hotkey key, QMenu *menu)
{
	menu->addAction(new abstract_menu_item(text, new T(type), toggle, key, this, false));
}

template <typename T, typename S>
void menu_manager::add_action(QString text, S type, hotkey key, QMenu *menu, bool check)
{
	menu->addAction(new abstract_menu_item(text, new T(type), nullptr, key, this, check));
}

template <typename T, typename S>
void menu_manager::add_group_action(QString text, S type, hotkey key, QMenu *menu, QActionGroup *group)
{
	menu->addAction(group->addAction((new abstract_menu_item(text, new T(type), nullptr, key, this, true))));
}

void menu_manager::create_actions(QUndoGroup *undo_group)
{
	toggle_function active_editors = &main_window::active_editors;
	
	QMenu *menu = find_menu("&File");
	add_action<window_event>("&New", NEW, hotkey::New, menu);
	add_action<window_event>("&Open", OPEN, hotkey::Open, menu);
	add_action<window_event>("&Save", SAVE, hotkey::Save, menu);
	add_action<window_event>("&Save as", SAVE_AS, hotkey::SaveAs, menu);
	menu->addSeparator();
	add_toggle_action<window_event>("&Close tab", CLOSE_TAB, active_editors, hotkey::Close, menu);
	add_action<window_event>("E&xit", CLOSE, hotkey::Quit, menu);

	menu = find_menu("&Edit");
	menu->addAction(new history_menu_item("U&ndo", new editor_event(UNDO), hotkey::Undo, this, undo_group));
	menu->addAction(new history_menu_item("R&edo", new editor_event(REDO), hotkey::Redo, this, undo_group));
	menu->addSeparator();
	add_toggle_action<editor_event>("Cu&t", CUT, &hex_editor::active_selection, hotkey::Cut, menu);
	add_toggle_action<editor_event>("&Copy", COPY, &hex_editor::active_selection, hotkey::Copy, menu);
	add_toggle_action<editor_event>("&Paste", PASTE, &hex_editor::clipboard_usable, hotkey::Paste, menu);
	add_toggle_action<editor_event>("&Delete", DELETE_TEXT, &hex_editor::active_selection, hotkey::Delete, menu);
	menu->addSeparator();
	add_toggle_action<editor_event>("&Select all", SELECT_ALL, active_editors, hotkey::SelectAll, menu);
	add_toggle_action<dialog_event>("Select &range", SELECT_RANGE, active_editors, hotkey("Ctrl+r"), menu);
	menu->addSeparator();
	add_toggle_action<dialog_event>("&Find/Replace", FIND_REPLACE, active_editors, hotkey("Ctrl+f"), menu);

	menu = find_menu("&Navigation");
	add_toggle_action<dialog_event>("&Goto offset", GOTO, active_editors, hotkey("Ctrl+g"), menu);
	
	menu = find_menu("&ROM utilities");
	add_toggle_action<dialog_event>("&Expand ROM", EXPAND, active_editors, hotkey("Ctrl+e"), menu);
	add_toggle_action<dialog_event>("&Metadata editor", METADATA_EDITOR, active_editors, hotkey("Ctrl+m"), menu);
	menu->addSeparator();
	add_toggle_action<editor_event>("Follow b&ranch", BRANCH, active_editors, hotkey("Alt+j"), menu);
	add_toggle_action<editor_event>("Follow &jump", JUMP, active_editors, hotkey("Ctrl+j"), menu);
	add_toggle_action<editor_event>("&Disassemble", DISASSEMBLE, active_editors, hotkey("Ctrl+d"), menu);
	add_toggle_action<editor_event>("&Bookmark", BOOKMARK, active_editors, hotkey("Ctrl+b"), menu);

	menu = find_menu("&Options");
	add_toggle_action<editor_event>("&Scrollbar toggle", SCROLL_MODE, active_editors, hotkey("Alt+s"), menu);
	add_action<dialog_event>("&Character map editor", MAP_EDITOR, hotkey("Alt+c"), menu);
	add_action<panel_event>("Disassembly panel toggle", DISASSEMBLER, hotkey("Alt+d"), menu, true);
	add_action<panel_event>("Bookmark panel toggle", BOOKMARKS, hotkey("Alt+b"), menu, true);
	menu->addSeparator();
	
	menu->addMenu(find_menu("&Copy style"));
	menu = find_menu("&Copy style");
	QActionGroup *copy_group = new QActionGroup(menu);
	add_group_action<editor_event>("&No space", NO_SPACES, hotkey("Alt+1"), menu, copy_group);
	add_group_action<editor_event>("&Spaces", SPACES, hotkey("Alt+2"), menu, copy_group);
	add_group_action<editor_event>("&Hex format", HEX_FORMAT, hotkey("Alt+3"), menu, copy_group);
	add_group_action<editor_event>("&Word table", ASM_WORD_TABLE, hotkey("Alt+5"), menu, copy_group);
	add_group_action<editor_event>("&Byte table", ASM_BYTE_TABLE, hotkey("Alt+4"), menu, copy_group);
	add_group_action<editor_event>("&Long table", ASM_LONG_TABLE, hotkey("Alt+6"), menu, copy_group);
	add_group_action<editor_event>("&C source", C_SOURCE, hotkey("Alt+7"), menu, copy_group);
	enable_checkable(copy_group);
	
	menu = find_menu("&Options");
	menu->addSeparator();
	add_action<dialog_event>("&Settings", SETTINGS, hotkey("Alt+o"), menu);

	menu = find_menu("&Help");
	add_action<window_event>("&Version", VERSION, hotkey("Alt+v"), menu);
}

QMenu *menu_manager::find_menu(QString id)
{
	for(auto &menu : menu_list){
		if(menu->title() == id){
			return menu;
		}
	}
	qDebug() << "Error: Menu " << id << " not found";
	return 0;
}

void menu_manager::enable_checkable(QActionGroup *group)
{
	QList<QAction *> actions = group->actions();
	for(auto &current : actions){
		current->setCheckable(true);
	}
	actions.first()->setChecked(true);
}

void menu_manager::post_event(QEvent *event)
{
	event_types event_type = (event_types)event->type();
	if(event_map.contains(event_type) && event_map[event_type]){
		QCoreApplication::sendEvent(event_map[event_type], event);
	}
}

void menu_manager::check_enabled(QMenu *menu)
{
	for(auto &action : menu->actions()){
		if(!action->isSeparator() && !action->menu()){
			dynamic_cast<abstract_menu_item *>(action)->check_enabled();
		}
	}
}

menu_manager::~menu_manager()
{
	for(auto &menu : menu_list){
		delete menu;
	}
}
