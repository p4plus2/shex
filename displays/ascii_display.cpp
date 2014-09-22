#include "ascii_display.h"
#include "character_mapper.h"
#include <QPainter>
#include <QTextStream>

#include "debug.h"

void ascii_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);

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
