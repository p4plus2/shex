#include "hex_display.h"
#include <QPainter>
#include <QTextStream>
#include <QStaticText>

#include "debug.h"

void hex_display::paintEvent(QPaintEvent *event)
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
		string_stream << " " 
		              << QString::number((unsigned char)buffer->at(i),16).rightJustified(2, '0').toUpper();
	}
	
	painter.drawStaticText(0,0, QStaticText(line));
}

QSize hex_display::sizeHint () const
{
	const int padding = 20;
	return QSize(get_font_width() * line_characters + padding, 200);
}
