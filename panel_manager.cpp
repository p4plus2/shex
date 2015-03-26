#include "panel_manager.h"
#include "panels/disassembler_panel.h"
#include "panels/bookmark_panel.h"
#include "hex_editor.h"

panel_manager::panel_manager(QWidget *parent) : QWidget(parent)
{
	panel_map[DISASSEMBLER] = new disassembler_panel(parent);
	panel_map[BOOKMARKS] = new bookmark_panel(parent);
	
	foreach(abstract_panel *panel, panel_map){
		layout->addWidget(panel->get_display());
		panel->toggle_display(panel->display_state());
	}
}

void panel_manager::init_displays()
{
	setLayout(layout);
	foreach(abstract_panel *panel, panel_map){
		panel->init_display();
	}	
}

bool panel_manager::event(QEvent *event)
{
	if(event->type() != (QEvent::Type)PANEL_EVENT){
		return QObject::event(event);
	}
	
	abstract_panel *panel = find_panel(((panel_event *)event)->sub_type());
	if(!event->isAccepted()){
		panel->toggle_state();
	}
	panel->toggle_display(panel->display_state());
	event->accept();
	return true;
}

void panel_manager::connect_to_editor(hex_editor *editor)
{
	connect(editor, &hex_editor::send_disassemble_data, 
	        (disassembler_panel *)find_panel(DISASSEMBLER), &disassembler_panel::disassemble);
	connect(editor, &hex_editor::send_bookmark_data, 
	        (bookmark_panel *)find_panel(BOOKMARKS), &bookmark_panel::create_bookmark);
}

abstract_panel *panel_manager::find_panel(panel_events id)
{
	if(panel_map.contains(id)){
		return panel_map[id];
	}
	qDebug() << "Error: Panel " << id << " not found";
	return 0;
}

