#include "text_display.h"
#include <QPainter>
#include <QMouseEvent>

#include "debug.h"

text_display::text_display(const ROM_buffer *b, QWidget *parent) :
        QWidget(parent), buffer(b)
{
	font_setup();
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
