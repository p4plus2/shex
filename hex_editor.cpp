#include "hex_editor.h"
#include "version.h"
#include <QPainter>
#include <QFontMetrics>
#include <QAction>
#include <QMenu>
#include <cctype>
#include "QDebug"

hex_editor::hex_editor(QWidget *parent) :
    QWidget(parent)
{
	buffer = new ROM_buffer;
	columns = 16;
	rows = 32;
	offset = 0;
	setFocusPolicy(Qt::WheelFocus);
	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(update_cursor()));
	timer->start(1000);
	scroll_timer = new QTimer(this);
	connect(scroll_timer, SIGNAL(timeout()), this, SLOT(auto_scroll_update()));
	cursor_state = false;
	font_setup();
	
	scroll_mode = false;

	vertical_offset = 6;
	vertical_shift = column_height(1);
	cursor_position.setY(vertical_offset);
	cursor_position.setX(column_width(11));
	is_dragging = false;
	
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
	    this, SLOT(context_menu(const QPoint&)));
	
	emit update_range(get_max_lines());
	emit update_status_text(get_status_text());
}

QSize hex_editor::minimumSizeHint() const
{
	return QSize(buffer->get_line(0, columns).length()*font_width,
	             rows*font_height+font_height+vertical_offset+vertical_shift);
}

void hex_editor::slider_update(int position)
{
	if(!scroll_mode){
		cursor_position.setY(cursor_position.y() + (offset - position * columns));
		int old_offset = offset;
		offset = position * columns;
		if(selection_active){
			selection_start.setY(selection_start.y()  - (offset - old_offset));
			selection_current.setY(selection_current.y()  - (offset - old_offset));
		}
		update();
	}else{
		position -= height() / 2;
		if(position < 0){
			scroll_direction = false;
			position = -position;
		}else if(position > 0){
			scroll_direction = true;
		}else{
			scroll_speed = INT_MAX;
			scroll_timer->setInterval(scroll_speed);
			return;
		}
		scroll_speed = qAbs(((position - (height() /2))-1) / 15);
		scroll_timer->setInterval(scroll_speed);
	}
}

void hex_editor::auto_scroll_update()
{
	if(is_dragging){
		if(!scroll_direction){
			update_selection(mouse_position.x(), mouse_position.y());
		}else{
			update_selection(mouse_position.x(), mouse_position.y());
		}
		return;
	}
	int scroll_factor = 1;
	if(scroll_speed < 5){
		scroll_factor = qAbs(scroll_speed - 20);
	}
	for(int i = 0; i < scroll_factor; i++){
		if(!scroll_direction){
			update_cursor_position(cursor_position.x(), cursor_position.y() - font_height, false);
		}else{
			update_cursor_position(cursor_position.x(), cursor_position.y() + font_height, false);
		}
	}
	update();
}

void hex_editor::control_auto_scroll(bool enabled)
{
	auto_scrolling = enabled;
	if(auto_scrolling){
		scroll_timer->start(scroll_speed);
	}else{
		scroll_timer->stop();
	}
}

void hex_editor::update_cursor()
{
	cursor_state = !cursor_state;
	update();
}

void hex_editor::context_menu(const QPoint& position)
{	
	QMenu menu;
	menu.addAction("Cut", this, SLOT(cut()), QKeySequence::Cut)->setDisabled(!selection_active);
	menu.addAction("Copy", this, SLOT(copy()), QKeySequence::Copy)->setDisabled(!selection_active);
	menu.addAction("Paste", this, SLOT(paste()), QKeySequence::Paste)->setDisabled(buffer->check_paste_data());
	menu.addAction("Delete", this, SLOT(delete_text()), QKeySequence::Delete)->setDisabled(!selection_active);
	menu.addSeparator();
	menu.addAction("Select all", this, SLOT(select_all()), QKeySequence::SelectAll);
	menu.addSeparator();
	menu.addAction("Disassemble", this, SLOT(disassemble()))->setDisabled(!selection_active);
	
	menu.exec(mapToGlobal(position));
}

void hex_editor::cut()
{
	int position[2];
	if(!get_selection_range(position)){
		return;
	}
	
	buffer->cut(position[0], position[1]);
	cursor_position = selection_start;
	selection_active = false;
}

void hex_editor::copy()
{
	int position[2];
	if(!get_selection_range(position)){
		return;
	}
	
	buffer->copy(position[0], position[1]);
}

void hex_editor::paste()
{
	int position[2];
	if(get_selection_range(position)){
		buffer->paste(position[0], position[1]);
		return;
	}
	if(cursor_position.x() % 3 != 1){
		update_cursor_position(cursor_position.x()+font_width, 
		                       cursor_position.y()-vertical_shift-font_height/2);
	}
	buffer->paste(get_buffer_position(cursor_position.x(), cursor_position.y()));
	selection_active = false;
}

void hex_editor::delete_text()
{
	int position[2];
	if(!get_selection_range(position)){
		buffer->delete_text(get_buffer_position(cursor_position.x(), cursor_position.y()));
	}else{
		buffer->delete_text(position[0], position[1]);	
		selection_active = false;
		cursor_position = selection_start;
	}
	emit update_range(get_max_lines());
	emit update_status_text(get_status_text());
	update();
}

void hex_editor::select_all()
{
	selection_start.setX(column_width(11));
	selection_start.setY(vertical_offset-offset);
	selection_current.setX(column_width(11+columns*3)-font_width);
	selection_current.setY(column_height(get_max_lines()+rows)-offset);
	selection_active = true;
}

void hex_editor::disassemble()
{
}

void hex_editor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.translate(0, vertical_shift);
	int hex_offset = column_width(11);
	
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
	
	for(int i = hex_offset; i < columns * column_width(3) + hex_offset; i += column_width(6)){
		painter.fillRect(i-1, vertical_offset, column_width(2)+2, 
		                 column_height(rows)+6, palette().color(QPalette::AlternateBase));
	}

	if(cursor_position.y() > 0 && cursor_position.y() < column_height(rows)+vertical_offset && !selection_active){
		QRect active_line(hex_offset-1, cursor_position.y()-1+vertical_offset, 
		                  columns*column_width(3)-font_width+2, font_height);
		painter.fillRect(active_line, palette().color(QPalette::Highlight));
		if(cursor_state){
			painter.fillRect(cursor_position.x(), cursor_position.y()-1+vertical_offset, 
			                 1, font_height, text);
			painter.fillRect(to_ascii_column(cursor_position.x()), cursor_position.y()-1+vertical_offset, 
			                 1, font_height, text);
		}
	}
	
	if(selection_active){
		paint_selection(painter);
	}

	int byte_count = rows * columns + offset;
	for(int i = offset; i < byte_count; i += columns){
		QString line = buffer->get_line(i, columns);
		painter.drawText(0, column_height((i-offset)/columns)+font_height+vertical_offset, line);
	}

}

void hex_editor::paint_selection(QPainter &painter)
{
	QPoint start_point = get_selection_point(selection_start);
	QPoint end_point = get_selection_point(selection_current);
	int hex_offset = column_width(11);
	
	if(end_point.y() == start_point.y()){
		QRect starting_line(start_point.x()-1, start_point.y()-1+vertical_offset, 
		                    end_point.x() - start_point.x(), font_height);
		painter.fillRect(starting_line, palette().color(QPalette::Highlight));
		
		starting_line.setLeft(to_ascii_column(start_point.x()));
		starting_line.setWidth((end_point.x() - start_point.x())/3);
		painter.fillRect(starting_line, palette().color(QPalette::Highlight));
	}else{
		int direction = end_point.y() > start_point.y() ? 
			        hex_offset-1-end_point.x() : 
			        columns*column_width(3)-font_width+2-end_point.x()+hex_offset;
		QRect ending_line(end_point.x()-1, end_point.y()-1+vertical_offset, 
		                  direction, font_height);
		painter.fillRect(ending_line, palette().color(QPalette::Highlight));	
		
		ending_line.setLeft(to_ascii_column(end_point.x()+font_width));
		ending_line.setWidth(direction/3);
		painter.fillRect(ending_line, palette().color(QPalette::Highlight));
		
		direction = end_point.y() < start_point.y() ? 
			    hex_offset-1-start_point.x():
			    columns*column_width(3)-font_width+2-start_point.x()+hex_offset; 
		QRect starting_line(start_point.x()-1, start_point.y()-1+vertical_offset, 
		                    direction, font_height);
		painter.fillRect(starting_line, palette().color(QPalette::Highlight));
		
		starting_line.setLeft(to_ascii_column(start_point.x()));
		starting_line.setWidth(direction/3);
		painter.fillRect(starting_line, palette().color(QPalette::Highlight));
		
		if(qAbs(end_point.y()-start_point.y()) > font_height){
			if(end_point.y() < start_point.y()){
				qSwap(end_point, start_point); 
			}
			QRect middle_line(hex_offset-1, start_point.y()+font_height-1+vertical_offset, 
					  column_width(columns*3)-font_width+2, 
					  end_point.y()-start_point.y()-font_height);
			painter.fillRect(middle_line, palette().color(QPalette::Highlight));
			
			middle_line.setLeft(to_ascii_column(hex_offset));
			middle_line.setWidth(column_width(columns));
			painter.fillRect(middle_line, palette().color(QPalette::Highlight));
		}	
	}	
}

void hex_editor::keyPressEvent(QKeyEvent *event)
{
	
	if(event->modifiers() == Qt::ControlModifier){
		switch(event->key()){
			case Qt::Key_X:
				cut();
			break;
			case Qt::Key_C:
				copy();
			break;
			case Qt::Key_V:
				paste();
			break;
			case Qt::Key_A:
				select_all();
			break;
		}
		emit update_range(get_max_lines());
		emit update_status_text(get_status_text());
		update();
		return;
	}
	
	if(event->modifiers() == Qt::AltModifier){
		switch(event->key()){
			case Qt::Key_S:
				scroll_mode = !scroll_mode;
				emit update_range(get_max_lines());
				emit toggle_scroll_mode(scroll_mode);
			break;
			case Qt::Key_V:
				display_version_dialog();
			break;
		}
		update();
		return;
	}
	
	if(event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9){
		if(selection_active){
			delete_text();
		}
		buffer->update_nibble(event->key() - Qt::Key_0, 
		                      get_buffer_position(cursor_position.x(), cursor_position.y(), false));
		update_cursor_position(cursor_position.x()+font_width, cursor_position.y());
	}else if(event->key() >= Qt::Key_A && event->key() <= Qt::Key_F){
		if(selection_active){
			delete_text();
		}
		buffer->update_nibble(event->key() - Qt::Key_A + 10, 
		                      get_buffer_position(cursor_position.x(), cursor_position.y(), false));
		update_cursor_position(cursor_position.x()+font_width, cursor_position.y());
	}

	switch(event->key()){
	        case Qt::Key_Delete:
			delete_text();
		break;
		case Qt::Key_Backspace:
			update_cursor_position(cursor_position.x()-column_width(3), cursor_position.y(), false);
			delete_text();
		break;

		case Qt::Key_Up:
			if(selection_active){
				update_selection_position(-columns);
			}else{
				update_cursor_position(cursor_position.x(), cursor_position.y() - font_height);
			}
		break;
		case Qt::Key_Down:
			if(selection_active){
				update_selection_position(columns);
			}else{
				update_cursor_position(cursor_position.x(), cursor_position.y() + font_height);
			}
		break;
		case Qt::Key_Right:
			update_cursor_position(cursor_position.x()+font_width, cursor_position.y());
		break;
		case Qt::Key_Left:
			update_cursor_position(cursor_position.x()-column_width(2), cursor_position.y());
		break;
		case Qt::Key_PageUp:
			update_cursor_position(cursor_position.x(), cursor_position.y() - column_height(rows));
		break;
		case Qt::Key_PageDown:
			update_cursor_position(cursor_position.x(), cursor_position.y() + column_height(rows));
		break;
		default:
		break;
	}
}

void hex_editor::wheelEvent(QWheelEvent *event)
{
	int steps = event->delta() / 8 / 15;
	int old_offset = offset;
	if(steps > 0 && offset > 0){
		if(offset - columns * steps < 0){
			offset = 0;
			cursor_position.setY(vertical_offset);
		}else{
			offset -= columns * steps;
			cursor_position.setY(cursor_position.y()+(column_height(steps)));
		}
	}
	if(steps < 0 && offset < buffer->size()){
		if((offset + columns * -steps) > buffer->size() - columns * rows){
			offset = buffer->size() - columns * rows;
			cursor_position.setY(column_height(rows-1)+vertical_offset);
		}else{
			offset += columns * -steps;
			cursor_position.setY(cursor_position.y()-(column_height(-steps)));
		}
	}
	if(selection_active && old_offset != offset){
		selection_start.setY(selection_start.y()  - (offset - old_offset));
		selection_current.setY(selection_current.y()  - (offset - old_offset));
	}
	if(!scroll_mode){
		emit update_slider(offset / columns);
	}
	emit update_status_text(get_status_text());
}

void hex_editor::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton){
		return;
	}
	
	int x = event->x();
	
	if((event->x() > column_width(3*columns+15) && event->x() < column_width(4*columns+15))){
		x = to_hex_column(x);
		click_side = true;
	}else{
		click_side = false;
	}
		
	selection_active = false;
	if(event->y() < vertical_offset+vertical_shift){
		event->ignore();
		return;
	}
	if(x > column_width(11) && x < column_width(11+columns*3)-font_width){
		update_cursor_position(x, event->y()-vertical_shift-font_height/2);
		if(!is_dragging){
			is_dragging = true;
			selection_start = cursor_position;
			if(cursor_position.x() % 3 != 1){
				selection_start.setX(cursor_position.x()-font_width);
			}
			selection_current = selection_start;
		}
	}
}

void hex_editor::mouseMoveEvent(QMouseEvent *event)
{
	mouse_position = event->pos();
	if(click_side){
		mouse_position.setX(to_hex_column(event->x()));
	}
	
	if(is_dragging){
		selection_active = true;
		update_selection(mouse_position.x(), event->y());
		if(event->y() > column_height(rows)){
			scroll_timer->start(20);
			scroll_direction = true;
		}else if(event->y() < vertical_shift){
			scroll_timer->start(20);
			scroll_direction = false;
		}else{
			scroll_timer->stop();
		}
	}
}

void hex_editor::mouseReleaseEvent(QMouseEvent *event)
{
	mouse_position = event->pos();
	if(click_side){
		mouse_position.setX(to_hex_column(event->x()));
	}
	
	is_dragging = false;
	scroll_timer->stop();
	if(selection_current == selection_start){
		selection_active = false;
		return;
	}
	if(is_dragging){
		update_selection(mouse_position.x(), event->y());
	}
}

void hex_editor::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	rows = (size().height() - vertical_shift - font_height)/ font_height;
	emit update_range(get_max_lines());
}

void hex_editor::font_setup()
{
	font.setFamily("Courier");
	font.setStyleHint(QFont::TypeWriter);
	font.setKerning(false);
	font.setPixelSize(14);
	
	QFontMetrics font_info(font);
	font_width = font_info.averageCharWidth();
	font_height = font_info.height();
}

QString hex_editor::get_status_text()
{
	QString text;
	QTextStream string_stream(&text);
	if(selection_active){
		int position1 = get_buffer_position(selection_start.x(), selection_start.y());
		int position2 = get_buffer_position(selection_current.x(), selection_current.y());
		if(position1 > position2){
			qSwap(position1, position2);
		}
		if(position1 < 0){
			position1 += offset;
		}
		
		string_stream << "Selection range: $" << buffer->get_address(position1)
			      << " to $" << buffer->get_address(position2);
	}else{
		int position = get_buffer_position(cursor_position.x(), cursor_position.y());
		unsigned char byte = buffer->at(position);
		
		string_stream << "Current offset: $" << buffer->get_address(position)
			      << "    Hex: 0x" << QString::number(byte, 16).rightJustified(2, '0').toUpper()
			      << "    Dec: " << QString::number(byte).rightJustified(3, '0');
	}
	return text;
}

QPoint hex_editor::get_selection_point(QPoint point)
{
	if(point.y() < 0){
		point.setY(vertical_offset);
		point.setX(column_width(11));
	}else if(point.y() > column_height(rows)+vertical_offset - font_height){
		point.setY(column_height(rows)+vertical_offset -font_height);
		point.setX(column_width(11+columns*3)-font_width);
	}
	return point;
}

bool hex_editor::get_selection_range(int position[2])
{
	position[0] = get_buffer_position(selection_start.x(), selection_start.y());
	position[1] = get_buffer_position(selection_current.x(), selection_current.y());
	if(position[0] > position[1]){
		qSwap(position[0], position[1]);
		qSwap(selection_start, selection_current);
	}
	return selection_active;
}

int hex_editor::get_buffer_position(int x, int y, bool byte_align)
{
	int position = (x - column_width(11)) / font_width;
	position -= position / 3;
	position = ((y-vertical_offset)/font_height)*columns*2+position+offset*2;
	return byte_align ? position/2 : position;

}

void hex_editor::update_cursor_position(int x, int y, bool do_update)
{
	int x_column = x - (x % font_width);
	if(x < column_width(11)-font_width){
		if(y < vertical_offset){
			x_column = column_width(11);
		}else{
			x_column = (columns - 1) * column_width(4) - column_width(3);
		}
		y -= font_height;
	}

	
	if(x_column > (columns - 1) * column_width(4) - column_width(3)){
		x_column = column_width(11);
		y += font_height;
	}

	if(y > rows * font_height){
		y -= font_height;
		if(offset < buffer->size() - columns * rows){
			offset += columns;
			if(!scroll_mode){
				emit update_slider(offset / columns);
			}
		}else{
			x_column = (columns - 1) * column_width(4) - column_width(3);
		}
	}
	if(y < 0 && offset > 0){
		y += font_height;
		offset -= columns;
		if(!scroll_mode){
			emit update_slider(offset / columns);
		}
	}
	if(y > 0 && y < column_height(rows)){
		cursor_position.setY(y - (y % font_height) + vertical_offset);
	}
	if(x_column % 3 != 2){
		cursor_position.setX(x_column);
	}else{
		cursor_position.setX(x_column + font_width);
	}
	cursor_state = true;
	if(do_update){
		update();
	}
	emit update_status_text(get_status_text());
}

void hex_editor::update_selection_position(int amount)
{
	int new_offset = offset + amount;
	if(new_offset > 0 && new_offset < buffer->size()){
		offset = new_offset;
		selection_start.setY(selection_start.y() - amount);
		selection_current.setY(selection_current.y() - amount);
	}
	update();
}

void hex_editor::update_selection(int x, int y)
{
	bool override = false;
	int old_offset = offset;
	if(x < column_width(11)){
		x = column_width(11);
	}else if(x >= column_width(10+columns*3)-font_width){
		x = column_width(8+columns*3)-font_width;
		override = true;
	}
	if(y < vertical_shift && offset == 0){
		x = column_width(11);
	}else if(!offset && y < vertical_shift){
		selection_current.setX(column_width(11));
		cursor_position = selection_current;
	}
	update_cursor_position(x, y-vertical_shift-font_height/2, false);
	if(cursor_position.x() % 3 != 1){
		update_cursor_position(x+font_width, y-vertical_shift-font_height/2, false);
	}
	selection_current = cursor_position;
	if(override){
		selection_current.setX(selection_current.x()+column_width(2));
		cursor_position = selection_current;
	}
	if(offset == buffer->size() - columns * rows && y > column_height(rows)){
		selection_current.setX(column_width(11+columns*3)-font_width);
		cursor_position = selection_current;
	}
	if(old_offset != offset){
		selection_start.setY(selection_start.y() - (offset - old_offset));
	}
	update();
	emit update_status_text(get_status_text());
}
