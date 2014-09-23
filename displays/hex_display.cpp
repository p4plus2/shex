#include "hex_display.h"
#include <QPainter>
#include <QTextStream>
#include <QMouseEvent>
#include <QKeyEvent>

#include "debug.h"

void hex_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);
}

void hex_display::mousePressEvent(QMouseEvent *event)
{
	QPoint position = map_to_byte(event->x(), event->y(), true);
	set_cursor_position(position.x(), position.y());
	qDebug() << position;
	update();
}

void hex_display::keyPressEvent(QKeyEvent *event)
{
	//		if(event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9){
	//			handle_typed_character(event->key() - Qt::Key_0);
	//		}else if(event->key() >= Qt::Key_A && event->key() <= Qt::Key_F){
	//			handle_typed_character(event->key() - Qt::Key_A + 10);
	//		}
	qDebug() << "hex" << event;
}

QSize hex_display::sizeHint () const
{
	const int padding = 20;
	return QSize(get_font_width() * line_characters + padding, get_font_height() * get_rows());
}


void hex_display::get_line(int start, int end, QTextStream &stream)
{
	for(int i = start; i < end; i++){
		stream << QString::number((unsigned char)buffer->at(i),16).rightJustified(2, '0').toUpper() << " ";
	}
}

QPoint hex_display::map_to_byte(int x, int y, bool byte_align)
{
	int x_remainder = x % get_font_width();
	x -= x_remainder;
	y -= y % get_font_height();
	if((x / get_font_width()) % 3 == 2){
		x += get_font_width() * ((x_remainder - get_font_width() / 2) < 0 ? -1 : 1);
	}
	
	int last_byte = (line_characters - 1) * get_font_width();
	if(x > last_byte){
		x = last_byte;
	}
	
	if(byte_align && (x / get_font_width()) % 3 == 1){
		x -= get_font_width();
	}
	
	return QPoint(x, y);
}
