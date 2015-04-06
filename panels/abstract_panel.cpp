#include "abstract_panel.h"
#include "panel_manager.h"
#include "utility.h"

abstract_panel::abstract_panel(panel_manager *display_parent, hex_editor *editor)
{
	display->setParent(display_parent);
	active_editor = editor;
}

void abstract_panel::toggle_display(bool state)
{
	display->setVisible(state);
	propagate_resize(display);
}

void abstract_panel::toggle_event(panel_events event)
{
	((panel_manager *)display->parent())->send_event(new panel_event(event));
}


