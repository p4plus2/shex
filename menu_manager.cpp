#include <QMenuBar>
#include "menu_manager.h"
#include "menus/history_menu_item.h"
#include "menus/generic_menu_item.h"
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
#define add_toggle_action(M,N,R,T,H) menu->addAction(new generic_menu_item<M *>(N, SLOT(R), SIGNAL(T), H, menu))
#define add_action(M,N,R,H) menu->addAction(new generic_menu_item<M *>(N, SLOT(R), "", H, menu))
#define add_history_action(N,R,H) menu->addAction(new history_menu_item(N, SLOT(R), "", H, menu))
	QMenu *menu = find_menu("&File");
	add_action(main_window, "&New", new_file(), hotkey::New);
	add_action(main_window, "&Open", open(), hotkey::Open);
	add_action(main_window, "&Save", save(), hotkey::Save);
	menu->addSeparator();
	add_action(main_window, "E&xit", close(), hotkey::Quit);

	menu = find_menu("&Edit");
	add_history_action("U&ndo", update_hex_editor(), hotkey::Undo);
	add_history_action("R&edo", update_hex_editor(), hotkey::Redo);
	menu->addSeparator();
	add_toggle_action(hex_editor, "Cu&t", cut(), selection_toggled(bool),hotkey::Cut);
	add_toggle_action(hex_editor, "&Copy", copy(), selection_toggled(bool), hotkey::Copy);
	add_toggle_action(hex_editor, "&Paste", paste(), clipboard_usable(bool), hotkey::Paste);
	add_toggle_action(hex_editor, "&Delete", delete_text(), selection_toggled(bool), hotkey::Delete);
	menu->addSeparator();
	add_toggle_action(hex_editor, "&Select all", select_all(), focused(bool), hotkey::SelectAll);
	add_toggle_action(dialog_manager, "Select &range", 
	                  show_select_range_dialog(), active_editors(bool), hotkey("Ctrl+r"));
	menu->addSeparator();
	add_toggle_action(dialog_manager, "&Find/Replace", 
	                  show_find_replace_dialog(), active_editors(bool), hotkey("Ctrl+f"));

	menu = find_menu("&Navigation");
	add_toggle_action(dialog_manager, "&Goto offset", show_goto_dialog(), active_editors(bool), hotkey("Ctrl+g"));
	
	menu = find_menu("&ROM utilities");
	add_toggle_action(dialog_manager, "&Expand ROM", show_expand_dialog(), active_editors(bool), hotkey("Ctrl+e"));
	add_toggle_action(dialog_manager, "&Metadata editor", 
	                  show_metadata_editor_dialog(), active_editors(bool), hotkey("Ctrl+m"));
	menu->addSeparator();
	add_toggle_action(hex_editor, "Follow &branch", branch(), focused(bool),hotkey("Ctrl+b"));
	add_toggle_action(hex_editor, "Follow &jump", jump(), focused(bool), hotkey("Ctrl+j"));
	add_toggle_action(hex_editor, "&Disassemble", disassemble(), focused(bool), hotkey("Ctrl+d"));

	menu = find_menu("&Options");
	add_toggle_action(hex_editor, "&Scrollbar toggle", scroll_mode_changed(), focused(bool), hotkey("Alt+s"));
	add_action(dialog_manager, "&Character map editor", show_map_editor_dialog(), hotkey("Alt+c"));
	
	menu = find_menu("&Help");
	add_action(main_window, "&Version", version(), hotkey("Alt+v"));

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
