#include "abstract_panel.h"

abstract_panel::abstract_panel(QWidget *display_parent)
{
	display->setParent(display_parent);
}

void abstract_panel::set_active_editor(hex_editor *editor)
{
	active_editor = editor;
}

void abstract_panel::toggle_display(bool state)
{
	display->setVisible(state);
	layout_adjust();
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

