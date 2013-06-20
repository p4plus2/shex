#include <QMenuBar>
#include "menu_manager.h"
#include "menus/editor_menu_item.h"
#include "menus/window_menu_item.h"
#include "menus/dialog_menu_item.h"
#include "debug.h"


menu_manager::menu_manager(QObject *parent, QMenuBar *m) :
        QObject(parent)
{
	menu_bar = m;
	create_menus();
	create_actions();
}

void menu_manager::create_menus()
{
	menu_list.append(new QMenu("&File"));
	menu_list.append(new QMenu("&Edit"));
	menu_list.append(new QMenu("&Navigation"));
	menu_list.append(new QMenu("&ROM utilities"));
	menu_list.append(new QMenu("&Options"));
	menu_list.append(new QMenu("&Help"));
	
	foreach(QMenu *menu, menu_list){
		menu_bar->addMenu(menu);
	}
	find_menu("&File");
}

void menu_manager::create_actions()
{
typedef QKeySequence hotkey;
	//temporary comment until toggles are ready
#define add_toggle_action(M,N,R,T,H) menu->addAction(new M##_menu_item(N, SLOT(R), ""/*SIGNAL(T)*/, H, menu));
#define add_action(M,N,R,H) menu->addAction(new M##_menu_item(N, SLOT(R), "", H, menu));
	QMenu *menu = find_menu("&File");
	add_action(window, "&New", new_file(), hotkey::New);
	add_action(window, "&Open", open(), hotkey::Open);
	add_action(window, "&Save", save(), hotkey::Save);
	menu->addSeparator();
	add_action(window, "E&xit", close(), hotkey::Quit);
	
	//this needs fixed as well.
	//QAction *undo_action = undo_group->createUndoAction(this);
	//QAction *redo_action = undo_group->createRedoAction(this);
	//undo_action->setShortcuts(hotkey::Undo);
	//connect(undo_action, SIGNAL(triggered()), this, SLOT(update_hex_editor()));
	//redo_action->setShortcuts(hotkey::Redo);
	//connect(redo_action, SIGNAL(triggered()), this, SLOT(update_hex_editor()));
	
	menu = find_menu("&Edit");
	menu->addSeparator();
	add_toggle_action(editor, "Cu&t", cut(), toggle(bool),hotkey::Cut);
	add_toggle_action(editor, "&Copy", copy(), toggle(bool), hotkey::Copy);
	add_toggle_action(editor, "&Paste", paste(), toggle(bool), hotkey::Paste);
	add_toggle_action(editor, "&Delete", delete_text(), toggle(bool), hotkey::Delete);
	menu->addSeparator();
	add_toggle_action(editor, "&Select all", select_all(), toggle(bool), hotkey::SelectAll);
	add_toggle_action(dialog, "Select &range", show_select_range_dialog(), toggle(bool), hotkey("Ctrl+r"));

	menu = find_menu("&Navigation");
	add_toggle_action(dialog, "&Goto offset", show_goto_dialog(), toggle(bool), hotkey("Ctrl+g"));
	
	menu = find_menu("&ROM utilities");
	add_toggle_action(dialog, "&Expand ROM", show_expand_dialog(), toggle(bool), hotkey("Ctrl+e"));
	add_toggle_action(dialog, "&Metadata editor", show_metadata_editor_dialog(), toggle(bool), hotkey("Ctrl+m"));
	menu->addSeparator();
	add_toggle_action(editor, "Follow &branch", branch(), toggle(bool),hotkey("Ctrl+b"));
	add_toggle_action(editor, "Follow &jump", jump(), toggle(bool), hotkey("Ctrl+j"));
	add_toggle_action(editor, "&Disassemble", disassemble(), toggle(bool), hotkey("Ctrl+d"));

	menu = find_menu("&Options");
	add_toggle_action(editor, "&Scrollbar toggle", scroll_mode_changed(), toggle(bool), hotkey("Alt+s"));
	
	menu = find_menu("&Help");
	add_toggle_action(window, "&Version", version(), toggle(bool), hotkey("Alt+v"));

#undef add_action
#undef add_toggle_action
}

QMenu *menu_manager::find_menu(QString id)
{
	foreach(QMenu *menu, menu_list){
		if(menu->title() == id){
			return menu;
		}
	}
	qDebug() << "Error: Menu " << id << " not found";
	return 0;
}

menu_manager::~menu_manager()
{
	foreach(QMenu *menu, menu_list){
		delete menu;
	}
}
