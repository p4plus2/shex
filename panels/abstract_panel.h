#ifndef ABSTRACT_PANEL_H
#define ABSTRACT_PANEL_H

#include <QWidget>

class hex_editor;

class abstract_panel
{
	public:
		abstract_panel(QWidget *display_parent);
		virtual QLayout *get_layout() = 0;
		void toggle_display(bool force = false);
		void set_active_editor(hex_editor *editor);
		
		void init_display(){ display->setLayout(get_layout()); }
		inline QWidget *get_display(){ return display; }
	protected:
		hex_editor *active_editor;
		
	private:
		QWidget *display = new QWidget;
		
		void layout_adjust();
};

#endif // ABSTRACT_PANEL_H
