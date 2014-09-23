#include "ascii_display.h"
#include "character_mapper.h"
#include <QPainter>
#include <QTextStream>
#include <QMouseEvent>
#include <QKeyEvent>

#include "debug.h"

void ascii_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);

}

void ascii_display::mousePressEvent(QMouseEvent *event)
{
	QPoint position = map_to_byte(event->x(), event->y());
	set_cursor_position(position.x(), position.y());
	qDebug() << position;
	update();
}

void ascii_display::keyPressEvent(QKeyEvent *event)
{
	//		if(event->key() >= Qt::Key_Space && event->key() <= Qt::Key_AsciiTilde){
	//			char key = event->text().at(0).unicode();
	//			handle_typed_character(character_mapper::decode(key), true);
	//		}
	qDebug() << "ascii" << event;
}

QSize ascii_display::sizeHint () const
{
	return QSize(get_font_width() * line_characters, 200);
}


void ascii_display::get_line(int start, int end, QTextStream &stream)
{
	for(int i = start; i < end; i++){
		if(isprint(character_mapper::encode(buffer->at(i)))){
			stream << (char)character_mapper::encode(buffer->at(i));
		}else{
			stream << ".";
		}
	}
}

QPoint ascii_display::map_to_byte(int x, int y)
{
	x -= x % get_font_width();
	y -= y % get_font_height();
	
	int last_byte = (line_characters - 1)* get_font_width();
	if(x > last_byte){
		x = last_byte;
	}
	
	return QPoint(x, y);
}
