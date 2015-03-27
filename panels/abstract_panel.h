#ifndef ABSTRACT_PANEL_H
#define ABSTRACT_PANEL_H

#include <QWidget>

#include "events/event_types.h"

class hex_editor;
class panel_manager;

class abstract_panel
{
	public:
		abstract_panel(panel_manager *display_parent, hex_editor *editor);
		virtual QLayout *get_layout() = 0;
		virtual void toggle_state() = 0;
		virtual bool display_state() = 0;
		void toggle_display(bool state);
		void toggle_event(panel_events event);
		
		void init_display(){ display->setLayout(get_layout()); }
		inline QWidget *get_display(){ return display; }
	protected:
		hex_editor *active_editor;
		
	private:
		QWidget *display = new QWidget;
		
		void layout_adjust();
};

#endif // ABSTRACT_PANEL_H
