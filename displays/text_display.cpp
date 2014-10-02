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
	if(!font_height){
		font_setup();
	}
	
	cursor_timer_id = startTimer(QApplication::cursorFlashTime());
	
	editor = parent;
	setFocusPolicy(Qt::WheelFocus);
	
	QSize minimum(0, 0);
	minimum.setHeight(rows*font_height);
	setMinimumSize(minimum);
}

void text_display::update_display()
{
	cursor_state = true;
	update();
}

void text_display::set_auto_scroll_speed(int speed)
{
	scroll_speed = speed;
	if(!speed && scroll_timer_id){
		killTimer(scroll_timer_id);
		scroll_timer_id = 0;
		scroll_speed = 1;
	}else if(!scroll_timer_id){
		scroll_timer_id = startTimer(scroll_timer_speed);
	}
}

QFont text_display::font_setup()
{
	font.setFamily("Courier");
	font.setStyleHint(QFont::TypeWriter);
	font.setKerning(false);
	font.setPixelSize(14);
	
	QFontMetrics font_info(font);
	font_width = font_info.averageCharWidth();
	font_height = font_info.height();
	return font;
}

QPoint text_display::clip_mouse(int x, int y)
{
	x = (x < 0 || y < 0) ? 0 : x;
	x = (x > width() || y > height()) ? width() : x;
	y = y < 0 ? 0 : y;
	y = y > height() ? height() : y;
	return QPoint(x, y);
}

void text_display::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
	
	selection selection_area = get_selection();
	QPoint cursor_position = nibble_to_screen(get_cursor_nibble() & ~selection_area.is_active());
	
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
	
	QPoint position1 = clip_screen(nibble_to_screen(selection_area.get_start() & ~1));
	QPoint position2 = clip_screen(nibble_to_screen(selection_area.get_end() & ~1));
	painter.fillRect(0, position1.y(), get_line_characters() * font_width, 
	                 position2.y() - position1.y() + font_height, 
			 palette().color(QPalette::Active, QPalette::Highlight));	
	if(position1.x()){
		painter.fillRect(0, position1.y(), position1.x(), font_height, palette().color(QPalette::Base));
	}
	
	if(position2.x() < width()){
		painter.fillRect(position2.x(), position2.y(),
		                 get_line_characters() * font_width - position2.x(), 
		                 font_height, palette().color(QPalette::Base));
	}	
}

void text_display::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton || focusPolicy() == Qt::NoFocus){
		return;
	}
	int nibble = screen_to_nibble(clip_mouse(event->x(), event->y()));
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
	int nibble = screen_to_nibble(clip_mouse(event->x(), event->y()));
	set_cursor_nibble(nibble);

	selection selection_area = get_selection();
	if(!scroll_timer_id){
		selection_area.set_end(nibble);
	}
	selection_area.set_dragging(true);
	set_selection(selection_area);
	
	if((event->y() > height() || event->y() < 0) && !scroll_timer_id){
		scroll_timer_id = startTimer(scroll_timer_speed);
		scroll_direction = event->y() > 0 ? 1 : -1;
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
	int nibble = screen_to_nibble(clip_mouse(event->x(), event->y()));
	set_cursor_nibble(nibble);
	
	selection selection_area = get_selection();
	if(selection_area.is_active()){
		selection_area.set_end(nibble);
		selection_area.set_dragging(false);
		set_selection(selection_area);
	}
	
	if(scroll_timer_id){
		killTimer(scroll_timer_id);
		scroll_timer_id	= 0;
	}
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
		set_offset(get_offset() + columns * scroll_direction * scroll_speed);
		if(selection_area.is_dragging()){
			selection_area.move_end(columns * 2 * scroll_direction);
			set_selection(selection_area);
		}
	}
	editor->update_window();
}

int text_display::columns = 16;
int text_display::rows = 32;
int text_display::font_height = 0;
int text_display::font_width = 0;
QFont text_display::font;
