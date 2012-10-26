#include "hex_editor.h"
#include <QPainter>
#include <QFontMetrics>
#include <QMessageBox>
#include <QAction>
#include <QMenu>
#include <cctype>
#include "QDebug"

hex_editor::hex_editor(QWidget *parent, QString file_name, QUndoGroup *undo_group) :
        QWidget(parent)
{
	buffer = new ROM_buffer(file_name);
	buffer->initialize_undo(undo_group);
	
	QTimer *cursor_timer = new QTimer(this);
	cursor_timer->start(1000);
	connect(cursor_timer, SIGNAL(timeout()), this, SLOT(update_cursor_state()));
	connect(scroll_timer, SIGNAL(timeout()), this, SLOT(auto_scroll_update()));
	
	font_setup();
	
	vertical_shift = column_height(1);
	cursor_position = get_byte_position(0);
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(context_menu(const QPoint&)));
	
	QSize minimum = minimumSizeHint();
	minimum.setHeight(rows/2*font_height+vertical_offset+vertical_shift);
	setMinimumSize(minimum);
	setFocusPolicy(Qt::WheelFocus);
}

QSize hex_editor::minimumSizeHint() const
{
	return QSize((14 + columns * 4)*font_width,
	             rows*font_height+vertical_offset+vertical_shift);
}

QString hex_editor::get_file_name()
{
	return buffer->get_file_name();
}

void hex_editor::set_focus()
{
	emit update_status_text(get_status_text());
	setFocus();
	buffer->set_active();
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

void hex_editor::scroll_mode_changed()
{
	scroll_mode = !scroll_mode;
	update_window();
	emit toggle_scroll_mode(scroll_mode);
}

void hex_editor::auto_scroll_update()
{
	if(is_dragging){
		update_selection(mouse_position.x(), mouse_position.y());
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

void hex_editor::update_cursor_state()
{
	cursor_state = !cursor_state;
	update();
}

void hex_editor::update_undo_action()
{
	if(get_buffer_position(cursor_position) > buffer->size()){
		cursor_position = get_byte_position(buffer->size());
	}
	selection_active = false;
	is_dragging = false;
	update_window();
}

void hex_editor::goto_offset(int address, bool mode)
{
	if(mode){
		address = ((address & 0x7f0000) >> 1) + (address & 0x7fff);
	}else{
		address = get_buffer_position(cursor_position) + address;
	}
	
	if(address > buffer->size() - 1){
		QMessageBox::warning(this, "Address error", "The address you specificed is larger than the file."
		                     "  Please check your input and try again.");
		return;
	}
	offset = address - (rows / 2) * columns;
	offset -= offset % columns;
	if(offset < 0){
		offset = 0;
	}else if(offset > buffer->size() - rows * columns){
		offset = buffer->size() - rows * columns;
	}
	cursor_position = get_byte_position(address);
	update_window();
}

void hex_editor::select_range(int start, int end, bool mode)
{
	if(mode){
		start = ((start & 0x7f0000) >> 1) + (start & 0x7fff);
		end = ((end & 0x7f0000) >> 1) + (end & 0x7fff);
	}else{
		start = get_buffer_position(cursor_position) + start;
		end = get_buffer_position(cursor_position) + end;
	}
	
	if(start > buffer->size() - 1 || end > buffer->size() - 1){
		QMessageBox::warning(this, "Address error", "One or more addresses are larger than the file."
		                     "  Please check your input and try again.");
		return;
	}
	end++;
	offset = start - (rows / 2) * columns;
	offset -= offset % columns;
	if(offset < 0){
		offset = 0;
	}else if(offset > buffer->size() - rows * columns){
		offset = buffer->size() - rows * columns;
	}
	selection_start = get_byte_position(start);
	selection_current = get_byte_position(end);
	selection_active = true;
	update_window();
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
	update_window();
}

void hex_editor::copy()
{
	int position[2];
	if(!get_selection_range(position)){
		return;
	}
	
	buffer->copy(position[0], position[1]);
	update_window();
}

void hex_editor::paste(bool raw)
{
	int position[2];
	if(get_selection_range(position)){
		buffer->paste(position[0], position[1], raw);
	}else{
		int size = buffer->paste(get_buffer_position(cursor_position), 0, raw);
		cursor_position = get_byte_position(get_buffer_position(cursor_position)+size);
		selection_active = false;
	}
	update_window();
}

void hex_editor::delete_text()
{
	int position[2];
	if(!get_selection_range(position)){
		buffer->delete_text(get_buffer_position(cursor_position));
	}else{
		buffer->delete_text(position[0], position[1]);	
		selection_active = false;
		cursor_position = selection_start;
	}
	update_window();
}

void hex_editor::select_all()
{
	selection_start = get_byte_position(0);
	selection_current = get_byte_position(buffer->size());
	selection_active = true;
	emit update_status_text(get_status_text());
	update();
}

void hex_editor::disassemble()
{
}

void hex_editor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.translate(0, vertical_shift);
	
	QColor text = palette().color(QPalette::WindowText);
	painter.setPen(text);
	painter.setFont(font);
	
	for(int i = hex_offset; i < columns * column_width(3) + hex_offset; i += column_width(6)){
		painter.fillRect(i-1, vertical_offset-font_height, column_width(2)+2, 
		                 column_height(rows+1)+6, palette().color(QPalette::AlternateBase));
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

	painter.drawText(0, 0, offset_header);
	int byte_count = rows * columns + offset;
	for(int i = offset; i < byte_count; i += columns){
		QString line = buffer->get_line(i, columns);
		painter.drawText(0, column_height((i-offset)/columns)+font_height+vertical_offset, line);
	}
	
}

void hex_editor::paint_selection(QPainter &painter)
{		
	int start = get_selection_point(selection_start);
	int end = get_selection_point(selection_current);
	if(start > end){
		qSwap(start, end);
	}
	
	for(; start < end+1; start++){
		QPoint position = get_byte_position(start);
		painter.fillRect(position.x()-1, position.y(), font_width*3, font_height+4, 
		                 palette().color(QPalette::Active, QPalette::Highlight));
		painter.fillRect(to_ascii_column(position.x()), position.y(), font_width, font_height+4, 
		                 palette().color(QPalette::Active, QPalette::Highlight));
	}
	
	painter.fillRect(column_width(10+columns*3)+1, 0, font_width, column_height(rows+1), 
	                 palette().color(QPalette::Background));
}

bool hex_editor::event(QEvent *e)
{
	if(e->type() == QEvent::KeyPress && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Tab){
		click_side = !click_side;
		return true;
	}
	return QWidget::event(e);
}

void hex_editor::keyPressEvent(QKeyEvent *event)
{
	
	if(event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)){
		if(event->key() == Qt::Key_V){
			paste(true);
			update();
		}
	}
	
	if(click_side){
		if(event->key() >= Qt::Key_Space && event->key() <= Qt::Key_AsciiTilde){
			char key = event->modifiers() != Qt::ShiftModifier &&
			                event->key() >= Qt::Key_A && event->key() <= Qt::Key_Z ? 
			                        event->key() + 32 : event->key();
			handle_typed_character(key, true);
		}
	}else{
		if(event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9){
			handle_typed_character(event->key() - Qt::Key_0);
		}else if(event->key() >= Qt::Key_A && event->key() <= Qt::Key_F){
			handle_typed_character(event->key() - Qt::Key_A + 10);
		}
	}
	
	switch(event->key()){
		case Qt::Key_Backspace:
			update_cursor_position(cursor_position.x()-column_width(3), cursor_position.y(), false);
			delete_text();
		break;
			
		case Qt::Key_Home:
			update_cursor_position(column_width(11), cursor_position.y());
		break;
		case Qt::Key_End:
				update_cursor_position(column_width(9+columns*3), cursor_position.y());
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

void hex_editor::handle_typed_character(unsigned char key, bool update_byte)
{
	int position[2];
	if(get_selection_range(position)){
		selection_active = false;
		cursor_position = selection_start;
	}else{
		position[1] = 0;
	}
	if(update_byte){
		buffer->update_byte(key, get_buffer_position(cursor_position), position[0], position[1]);
	}else{
		buffer->update_nibble(key, get_buffer_position(cursor_position, false), position[0], position[1]);
	}
	update_cursor_position(cursor_position.x()+font_width*(2 * update_byte ? 2 : 1), cursor_position.y(), false);
	update_window();
}

void hex_editor::wheelEvent(QWheelEvent *event)
{
	int steps = event->delta() / 8 / 15;
	if(selection_active){
		update_selection_position(-steps * columns);
	}else{
		update_cursor_position(cursor_position.x(), cursor_position.y() + (-steps * font_height));
	}
}

void hex_editor::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::RightButton){
		return;
	}
	
	int x = event->x();
	int y = event->y() - vertical_shift;
	
	if((event->x() > column_width(3*columns+15) && event->x() < column_width(4*columns+15))){
		x = to_hex_column(x);
		click_side = true;
	}else{
		click_side = false;
	}
	if(get_buffer_position(x, y) > buffer->size() || event->y() < vertical_offset+vertical_shift){
		return;
	}
	
	selection_active = false;
	if(x > column_width(11) && x < column_width(11+columns*3)-font_width){
		update_cursor_position(x, y);
		is_dragging = true;
		selection_start = get_byte_position(get_buffer_position(x, y));
		selection_current = selection_start;
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

	if(is_dragging){
		update_selection(mouse_position.x(), event->y());
	}
}

void hex_editor::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);
	rows = (size().height() - vertical_shift)/ font_height;
	update_window();
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
	hex_offset = column_width(11);
	ascii_offset = column_width(15+columns*3);
}

QString hex_editor::get_status_text()
{
	QString text;
	QTextStream string_stream(&text);
	if(selection_active){
		int position1 = get_buffer_position(selection_start);
		int position2 = get_buffer_position(selection_current);
		if(position1 > position2){
			qSwap(position1, position2);
		}
		if(position1 < 0){
			position1 += offset;
		}
		
		string_stream << "Selection range: $" << buffer->get_address(position1)
		              << " to $" << buffer->get_address(position2);
	}else{
		int position = get_buffer_position(cursor_position);
		unsigned char byte = buffer->at(position);
		
		string_stream << "Current offset: $" << buffer->get_address(position)
		              << "    Hex: 0x" << QString::number(byte, 16).rightJustified(2, '0').toUpper()
		              << "    Dec: " << QString::number(byte).rightJustified(3, '0');
	}
	return text;
}

int hex_editor::get_selection_point(QPoint point)
{
	if(point.y() < 0){
		point.setY(vertical_offset);
		point.setX(column_width(11));
	}else if(point.y() > column_height(rows)+vertical_offset - font_height){
		point.setY(column_height(rows)+vertical_offset -font_height);
		point.setX(column_width(11+columns*3)-font_width);
	}
	return get_buffer_position(point);
}

bool hex_editor::get_selection_range(int position[2])
{
	position[0] = get_buffer_position(selection_start);
	position[1] = get_buffer_position(selection_current);
	if(position[0] > position[1]){
		qSwap(position[0], position[1]);
		qSwap(selection_start, selection_current);
	}
	position[1]++;
	return selection_active;
}

int hex_editor::get_buffer_position(QPoint &point, bool byte_align)
{
	return get_buffer_position(point.x(), point.y(), byte_align);
}

int hex_editor::get_buffer_position(int x, int y, bool byte_align)
{
	int position = (x - column_width(11)) / font_width;
	position -= position / 3;
	position = ((y-vertical_offset)/font_height)*columns*2+position+offset*2;
	return byte_align ? position/2 : position;
	
}

QPoint hex_editor::get_byte_position(int address, bool byte_align)
{
	int nibble_offset = 0;
	if(!byte_align){
		nibble_offset += (address & 1) * font_width;
		address /= 2;
	}
	int screen_relative = address - offset;
	return QPoint(column_width((screen_relative % columns)*3+11)+nibble_offset, 
	              column_height(screen_relative / columns) + vertical_offset);
}

void hex_editor::update_cursor_position(int x, int y, bool do_update)
{
	if(get_buffer_position(x, y) == buffer->size() && column_width(10+columns*3) == x){
		cursor_position.setX(x);
	}else if(x == column_width(11+columns*3)){
		cursor_position.setX(column_width(12));
		if(y + font_height > column_height(rows)){
			offset += columns;
		}else{
			cursor_position.setY(y + font_height);
		}
	}else{
		int position = get_buffer_position(x, y, false);
		if(position < 0){
			position = 0;
		}else if(position/2 > buffer->size()){
			position = buffer->size()*2-1;
		}
		while(position/2 >= offset + rows * columns){
			offset += columns;
		}
		while(position/2 < offset){
			offset -= columns;
		}
		cursor_position = get_byte_position(position, false);
	}
	
	if(do_update){
		update_window();
	}
}

void hex_editor::update_selection_position(int amount)
{
	int new_offset = offset + amount;
	if(new_offset > 0 && new_offset < buffer->size()){
		offset = new_offset;
		selection_start -= QPoint(0, amount);
		selection_current -= QPoint(0, amount);
	}
	update();
}

void hex_editor::update_selection(int x, int y)
{
	int old_offset = offset;
	QPoint last_byte = get_byte_position(buffer->size()-1);
	if(x < column_width(11)){
		x = column_width(11);
	}else if(x >= column_width(10+columns*3)-font_width){
		x = column_width(10+columns*3)-font_width*2;
	}else if(x >= last_byte.x()-font_width && y-font_height >= last_byte.y()){
		x = last_byte.x()-font_width*2;
	}
	x -= font_width;
	
	if(y < vertical_offset && !offset){
		y = vertical_offset;
	}else if(y > vertical_offset + column_height(rows) && offset == buffer->size()-rows*columns){
		y = vertical_offset + column_height(rows);
	}
	
	QPoint position = get_byte_position(get_buffer_position(x+font_width, y-vertical_shift));
	update_cursor_position(position.x(), position.y(), false);
	
	if(old_offset != offset){
		selection_start.setY(selection_start.y() - (offset - old_offset));
	}
	selection_current = cursor_position;
	update_window();
}

void hex_editor::update_window()
{
	if(!scroll_mode){
		emit update_slider(offset / columns);
	}
	emit update_range(get_max_lines());
	emit update_status_text(get_status_text());
	cursor_state = true;
	update();
}

hex_editor::~hex_editor()
{
	delete buffer;
}

const QString hex_editor::offset_header = "Offset     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";
