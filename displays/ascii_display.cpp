#include "ascii_display.h"
#include "character_mapper.h"
#include <QPainter>
#include <QTextStream>
#include <QStaticText>

#include "debug.h"

void ascii_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	set_painter_font(painter);
	
	int index = 0;
	int length = 16;
	
	QString line;
	if(index >= buffer->size()){
		return;
	}
	QTextStream string_stream(&line);

	int line_length = index+length;	
	if(line_length > buffer->size()){
		line_length = buffer->size();
	}
	
	for(int i = index; i < line_length; i++){
		if(isprint(character_mapper::encode(buffer->at(i)))){
			string_stream << (char)character_mapper::encode(buffer->at(i));
		}else{
			string_stream << ".";
		}
	}
	
	painter.drawStaticText(0,0, QStaticText(line));

}


QSize ascii_display::sizeHint () const
{
	return QSize(get_font_width() * line_characters, 200);
}
