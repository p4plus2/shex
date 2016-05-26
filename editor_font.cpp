#include <QFontDatabase>
#include <QFontMetrics>

#include "editor_font.h"
#include "settings_manager.h"
#include "debug.h"

editor_font::editor_font(QObject *parent) : QObject(parent)
{
	font_setup();
	settings_manager::add_persistent_listener(this, "display/font");
	self = this;
}

bool editor_font::event(QEvent *event)
{
	if(event->type() == (QEvent::Type)SETTINGS_EVENT){
		settings_event *e = (settings_event *)event;
		if(e->data().first == "display/font"){
			default_font_size = e->data().second.toInt();
			font_setup();
			emit font_changed();
		}
	}
	
	return true;
}

void editor_font::font_setup()
{
	font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	font.setPointSize(default_font_size);
	
	QFontMetricsF font_info(font);
	font_width = font_info.averageCharWidth();
	
	font_height = font_info.height();
}

float editor_font::font_height = 0;
float editor_font::font_width = 0;
int editor_font::default_font_size = 9;
QFont editor_font::font;
editor_font *editor_font::self = nullptr;
