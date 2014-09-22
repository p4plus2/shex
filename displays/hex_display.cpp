#include "hex_display.h"
#include <QPainter>
#include <QTextStream>

#include "debug.h"

void hex_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);
}

QSize hex_display::sizeHint () const
{
	const int padding = 20;
	return QSize(get_font_width() * line_characters + padding, get_font_height() * get_rows());
}


void hex_display::get_line(int start, int end, QTextStream &stream)
{
	for(int i = start; i < end; i++){
		stream << " " << QString::number((unsigned char)buffer->at(i),16).rightJustified(2, '0').toUpper();
	}
}
