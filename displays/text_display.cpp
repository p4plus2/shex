#include "text_display.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStaticText>

#include "debug.h"

text_display::text_display(const ROM_buffer *b, QWidget *parent) :
        QWidget(parent), buffer(b)
{
	font_setup();
}

void text_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	set_painter_font(painter);
	int index = 0;
	
	if(index >= buffer->size()){
		return;
	}
	
	int byte_count = get_rows() * get_columns() + index;
	for(int i = index, row = 0; i < byte_count; i += get_columns(), row++){
		int line_end = i + get_columns();	
		if(line_end > buffer->size()){
			line_end = buffer->size();
		}
		QString line;
		QTextStream string_stream(&line);
		get_line(i, line_end, string_stream);
		painter.drawStaticText(0, row * get_font_height(), QStaticText(line));
	}
}

void text_display::font_setup()
{
	font.setFamily("Courier");
	font.setStyleHint(QFont::TypeWriter);
	font.setKerning(false);
	font.setPixelSize(14);
	
	QFontMetrics font_info(font);
	font_width = font_info.averageCharWidth();
	font_height = font_info.height();
}

int text_display::get_font_width() const
{
	return font_width;
}

int text_display::get_font_height() const
{
	return font_height;
}

int text_display::get_rows() const
{
	return rows;
}

int text_display::get_columns() const
{
	return columns;
}

void text_display::set_painter_font(QPainter &painter)
{
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
}

void text_display::mouseMoveEvent(QMouseEvent *event)
{
	qDebug() << event->y();
}
