#include <QPainter>
#include <QSettings>

#include "debug.h"
#include "text_display.h"
#include "hex_editor.h"
#include "settings_manager.h"

text_display::text_display(const ROM_buffer *b, hex_editor *parent) :
        QWidget(parent), buffer(b)
{
	if(!font_height){
		font_setup();
	}
	
	cursor_timer_id = startTimer(QApplication::cursorFlashTime());
	
	editor = parent;
	setFocusPolicy(Qt::WheelFocus);
	
	setAttribute(Qt::WA_StaticContents, true);
	
	settings_manager::add_persistent_listener(this, "display/font");
	settings_manager::add_listener(this, "display/highlight");
	selection_color = QApplication::palette().color(QPalette::Active, QPalette::Highlight).lighter();
}

void text_display::update_display()
{
	cursor_state = true;
	invalidate_cache();
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

void text_display::font_setup()
{
	font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	font.setPointSize(default_font_size);
	
	QFontMetrics font_info(font);
	font_width = font_info.averageCharWidth();
	font_height = font_info.height();
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
	QPainter painter(this);
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
	painter.setClipping(true);
	
	const bookmark_map *bookmarks = buffer->get_bookmark_map();
	if(bookmarks){
		for(const auto &bookmark : *bookmarks){
			selection bookmark_selection = selection::create_selection(
							buffer->snes_to_pc(bookmark.address), bookmark.size);
			paint_selection(painter, bookmark_selection, bookmark.color);
		}
	}
	
	selection selection_area = get_selection();
	
	selection_color.setAlpha(170);
	if(selection_area.is_active()){
		paint_selection(painter, selection_area, selection_color);
	}
	
	painter.setClipRegion(event->region());
	
	if(!selection_area.is_active()){
		painter.setClipping(false);
		QPoint cursor_position = nibble_to_screen(get_cursor_nibble());
		if(cursor_state && focusPolicy() != Qt::NoFocus){
			painter.fillRect(cursor_position.x(), cursor_position.y(), cursor_width, font_height, text);
		}
		QRect active_line(0, cursor_position.y(), get_line_characters() * font_width, font_height);
		painter.fillRect(active_line, selection_color);
	}

	int byte_count = get_rows() * get_columns() + get_offset();
	for(int i = get_offset(), row = 0; i < byte_count; i += get_columns(), row++){
		int real_row = i / get_columns();
		if(!row_cache.contains(real_row)){
			int line_end = i + get_columns();	
			if(line_end > buffer->size()){
				line_end = buffer->size();
			}
			QString line;
			QTextStream string_stream(&line);
			get_line(i, line_end, string_stream);
			QStaticText *text = new QStaticText(line);
			text->setTextFormat(Qt::PlainText);
			row_cache.insert(real_row, text);
		}
		if(row * font_height >= event->rect().y()){
			painter.drawStaticText(0, row * font_height, *row_cache.object(real_row));
		}
	}
}

void text_display::paint_selection(QPainter &painter, selection &selection_area, const QColor &color)
{
	if(focusPolicy() == Qt::NoFocus){
		return;  //Anything which doesn't accept focus can't be highlighted
	}
	QPoint position1 = clip_screen(nibble_to_screen(selection_area.get_start_aligned()));
	QPoint position2 = clip_screen(nibble_to_screen(selection_area.get_end_aligned()));
	QRegion area = QRect(0, position1.y(), get_line_characters() * font_width, 
	                 position2.y() - position1.y() + font_height);
	if(position1.x()){
		area -= QRect(0, position1.y(), position1.x(), font_height);
	}
	
	if(position2.x() < width()){
		area -= QRect(position2.x() - font_width, position2.y(), 
		              (get_line_characters() + 1) * font_width - position2.x(), font_height);
	}
	painter.setClipRegion(area);
	painter.fillRect(0, position1.y(), get_line_characters() * font_width, 
	                 position2.y() - position1.y() + font_height, color);
}

bool text_display::event(QEvent *event)
{
	if(event->type() == (QEvent::Type)SETTINGS_EVENT){
		settings_event *e = (settings_event *)event;
		qDebug() << e->data().first << e->data().second;
		if(e->data().first == "display/font"){
			default_font_size = e->data().second.toInt();
			font_setup();
			update_size();
		}else if(e->data().first == "display/highlight"){
			selection_color = e->data().second.value<QColor>();
		}
		
		return true;
	}
	return QWidget::event(event);
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
		QPoint screen = nibble_to_screen(get_cursor_nibble());
		update(screen.x(), screen.y(), cursor_width, font_height);
	}else if(event->timerId() == scroll_timer_id){
		selection selection_area = get_selection();
		set_offset(get_offset() + columns * scroll_direction * scroll_speed);
		if(selection_area.is_dragging()){
			selection_area.move_end(columns * 2 * scroll_direction);
			set_selection(selection_area);
		}
		editor->update_window();
	}
}

void text_display::update_size()
{
	setMinimumWidth(font_width * get_line_characters());
	qDebug() << font_width;
	parentWidget()->layout()->invalidate();
        QWidget *parent = parentWidget();
        while (parent) {
		parent->adjustSize();
		parent = parent->parentWidget();
        }
	
}

int text_display::columns = 16;
int text_display::rows = 32;
int text_display::font_height = 0;
int text_display::font_width = 0;
QFont text_display::font;

int text_display::default_font_size = 9;
QColor text_display::selection_color;
