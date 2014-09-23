#include "text_display.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStaticText>
#include <QTimer>

#include "debug.h"

text_display::text_display(const ROM_buffer *b, QWidget *parent) :
        QWidget(parent), buffer(b)
{
	font_setup();
	
	QTimer *cursor_timer = new QTimer(this);
	cursor_timer->start(QApplication::cursorFlashTime());
	connect(cursor_timer, SIGNAL(timeout()), this, SLOT(update_cursor_state()));
}

void text_display::update_cursor_state()
{
	cursor_state = !cursor_state;
	update();
}

void text_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
	
	//index is temporary
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
	
	if(cursor_state && display_cursor){
		painter.fillRect(cursor_position.x(), cursor_position.y(), 1, font_height, text);
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

void text_display::set_cursor_position(int x, int y)
{
	cursor_position.setX(x);
	cursor_position.setY(y);
}

void text_display::mouseMoveEvent(QMouseEvent *event)
{
	qDebug() << event->y();
}
