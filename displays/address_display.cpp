#include "address_display.h"
#include "debug.h"
#include <QPainter>
#include <QTextStream>

address_display::address_display(const ROM_buffer *b, hex_editor *parent) : text_display(b, parent)
{
	setMaximumWidth(get_font_width() * line_characters);
	disable_cursor();
	setFocusPolicy(Qt::NoFocus);
}

void address_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);
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

//We only need Y position here
QPoint address_display::nibble_to_screen(int nibble)
{
	int y = nibble / (get_columns() * 2);
	return QPoint(0, y * get_font_height());
}
