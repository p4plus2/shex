#include "text_display.h"
#include "hex_editor.h"
#include <QPainter>
#include <QMouseEvent>
#include <QStaticText>
#include <QTimer>
#include <QTimerEvent>

#include "debug.h"

text_display::text_display(const ROM_buffer *b, hex_editor *parent) :
        QWidget(parent), buffer(b)
{
	font_setup();
	
	cursor_timer_id = startTimer(QApplication::cursorFlashTime());
	
	editor = parent;
	setFocusPolicy(Qt::WheelFocus);
}

void text_display::update_display()
{
	cursor_state = true;
	update();
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

int text_display::clip_x(int x)
{
	x = x < 0 ? 0 : x;
	return x > width() ? width() : x;
}

int text_display::clip_y(int y)
{
	y = y < 0 ? 0 : y;
	return y > height() ? height() : y;
}

void text_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
	
//	for(int i = hex_offset; i < total_byte_column_width + hex_offset; i += byte_column_width * 2){
//		painter.fillRect(i-1, 0, column_width(2)+2, 
//		                 column_height(rows+1)+vertical_offset, palette().color(QPalette::AlternateBase).darker());
//	}
	
	QPoint cursor_position = nibble_to_screen(get_cursor_nibble());
	
	selection selection_area = get_selection();
	if(!selection_area.is_active()){
		QRect active_line(0, cursor_position.y(), get_line_characters() * font_width, font_height);
		painter.fillRect(active_line, palette().color(QPalette::Highlight).lighter());
	}else{
		paint_selection(painter, selection_area);
	}
	
	int byte_count = get_rows() * get_columns() + get_offset();
	for(int i = get_offset(), row = 0; i < byte_count; i += get_columns(), row++){
		int line_end = i + get_columns();	
		if(line_end > buffer->size()){
			line_end = buffer->size();
		}
		QString line;
		QTextStream string_stream(&line);
		get_line(i, line_end, string_stream);
		painter.drawStaticText(0, row * font_height, QStaticText(line));
	}
	
	if(cursor_state && focusPolicy() != Qt::NoFocus){
		painter.fillRect(cursor_position.x(), cursor_position.y(), 1, font_height, text);
	}
}

void text_display::paint_selection(QPainter &painter, selection &selection_area)
{
	if(focusPolicy() == Qt::NoFocus){
		return;  //Anything which doesn't accept focus can't be highlighted
	}
	int start = selection_area.get_start();
	int end = selection_area.get_end();
	for(; start <= end && end != get_offset(); start++){
		QPoint position = nibble_to_screen(start & ~1);
		painter.fillRect(position.x(), position.y(), font_width, font_height, 
				 palette().color(QPalette::Active, QPalette::Highlight));
	}
}

void text_display::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton || focusPolicy() == Qt::NoFocus){
		return;
	}
	int nibble = screen_to_nibble(clip_x(event->x()), clip_y(event->y()));
	set_cursor_nibble(nibble);
	
	selection selection_area = get_selection();
	selection_area.set_start(nibble);
	selection_area.set_active(false);
	selection_area.set_dragging(false);
	set_selection(selection_area);
}

void text_display::mouseMoveEvent(QMouseEvent *event)
{
	if(focusPolicy() == Qt::NoFocus){
		return;
	}
	int nibble = screen_to_nibble(clip_x(event->x()), clip_y(event->y()));
	set_cursor_nibble(nibble);
	
	selection selection_area = get_selection();
	selection_area.set_end(nibble);
	selection_area.set_active(true);
	selection_area.set_dragging(true);
	set_selection(selection_area);
	
	if((event->y() > height() || event->y() < 0) && !scroll_timer_id){
		scroll_timer_id = startTimer(20);
	}else if(!(event->y() > height() || event->y() < 0) && scroll_timer_id){
		killTimer(scroll_timer_id);
		scroll_timer_id = 0;
	}
}

void text_display::mouseReleaseEvent(QMouseEvent *event)
{
	if(focusPolicy() == Qt::NoFocus){
		return;
	}
	int nibble = screen_to_nibble(clip_x(event->x()), clip_y(event->y()));
	set_cursor_nibble(nibble);
	
	selection selection_area = get_selection();
	selection_area.set_end(nibble);
	selection_area.set_dragging(false);
	set_selection(selection_area);
	
	if(scroll_timer_id){
		killTimer(scroll_timer_id);
		scroll_timer_id	= 0;
	}
	qDebug() << selection_area.get_start() << selection_area.get_end();
}

void text_display::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);

	rows = size().height() / font_height; 
}

void text_display::timerEvent(QTimerEvent *event)
{
	if(event->timerId() == cursor_timer_id){
		cursor_state = !cursor_state;
	}else if(event->timerId() == scroll_timer_id){
		selection selection_area = get_selection();
		//TODO
		//scroll direction also clip to region
		set_offset(get_offset() + columns);
		if(selection_area.is_dragging()){
			selection_area.move_end(columns * 2);
		}
		qDebug() << "offset" << get_offset();
		set_selection(selection_area);
	}
	update();
}
