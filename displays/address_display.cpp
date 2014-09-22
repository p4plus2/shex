#include "address_display.h"
#include "debug.h"
#include <QPainter>
#include <QTextStream>

void address_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);
	setMaximumWidth(get_font_width() * line_characters);
}


QSize address_display::sizeHint () const
{
	return QSize(get_font_width() * line_characters, 200);
}

void address_display::get_line(int start, int end, QTextStream &stream)
{
	Q_UNUSED(end);
	stream << "$" << buffer->get_formatted_address(start) << ": ";
}
