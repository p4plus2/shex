#include "abstract_panel.h"
#include "panel_manager.h"

abstract_panel::abstract_panel(panel_manager *display_parent, hex_editor *editor)
{
	display->setParent(display_parent);
	active_editor = editor;
}

void abstract_panel::toggle_display(bool state)
{
	display->setVisible(state);
	layout_adjust();
}

void abstract_panel::toggle_event(panel_events event)
{
	((panel_manager *)display->parent())->send_event(new panel_event(event));
}

void abstract_panel::layout_adjust()
{
        QWidget *parent = display;
        while(parent){
		int height = parent->height();
		parent->setUpdatesEnabled(false);
		parent->adjustSize();
		parent->resize(parent->width(), height);
		parent->setUpdatesEnabled(true);
		parent->repaint();
		parent = parent->parentWidget();
	}
}

