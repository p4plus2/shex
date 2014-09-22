#include "address_display.h"
#include "debug.h"
#include <QPainter>
#include <QTextStream>
#include <QStaticText>

void address_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	set_painter_font(painter);
	
	int index = 0;
	
	if(index >= buffer->size()){
		return;
	}
	
	QString line;
	QTextStream string_stream(&line);
	
	string_stream << "$" << buffer->get_formatted_address(index) << ": ";
	
	painter.drawStaticText(0,0, QStaticText(line));
	
	//todo: remove this hack
	setMaximumWidth(get_font_width() * line_characters);
}


QSize address_display::sizeHint () const
{
	return QSize(get_font_width() * line_characters, 200);
}
