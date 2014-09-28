#include "hex_editor.h"
#include "character_mapper.h"
#include "displays/hex_display.h"
#include "displays/ascii_display.h"
#include "displays/address_display.h"
#include "debug.h"

#include <QPainter>
#include <QFontMetrics>
#include <QMessageBox>
#include <QAction>
#include <QMenu>

#include <QHBoxLayout>

hex_editor::hex_editor(QWidget *parent, QString file_name, QUndoGroup *undo_group, bool new_file) :
        QWidget(parent)
{
	buffer = new ROM_buffer(file_name, new_file);
	if(buffer->load_error() != ""){
		ROM_error = buffer->load_error();
		return;
	}
	buffer->initialize_undo(undo_group);
	is_new = new_file;
	
	if(new_file){
		update_save_state(1);
	}
	
	
	connect(scroll_timer, SIGNAL(timeout()), this, SLOT(auto_scroll_update()));
	
	vertical_shift = column_height(1);
	cursor_position = get_byte_position(0);
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(context_menu(const QPoint&)));
	connect(qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboard_changed()));
	
	font_width = 10; //stop refactor crashes
	font_height = 10;
	
	QSize minimum(0, 0);
	minimum.setHeight(rows*font_height);
	setMinimumSize(minimum);
	
	//can't initialize in header -- relies on buffer here to be const
	address = new address_display(buffer, this);
	hex = new hex_display(buffer, this);
	ascii = new ascii_display(buffer, this);
	
	connect(hex, SIGNAL(character_typed(unsigned char, bool)), 
	        this, SLOT(handle_typed_character(unsigned char, bool)));
	connect(ascii, SIGNAL(character_typed(unsigned char, bool)), 
	        this, SLOT(handle_typed_character(unsigned char, bool)));
	
	QHBoxLayout *layout = new QHBoxLayout();
	layout->addWidget(address);
	layout->addWidget(hex);
	layout->addWidget(ascii);
	setLayout(layout);
}

void hex_editor::set_focus()
{
	emit update_status_text(get_status_text());
	hex->setFocus();
	buffer->set_active();
	emit selection_toggled(selection_active);
	emit focused(true);
	emit update_save_state(0);
	clipboard_changed();
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
			move_cursor_nibble(-columns * 2);
		}else{
			move_cursor_nibble(columns * 2);
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

void hex_editor::handle_typed_character(unsigned char key, bool update_byte)
{
	int start, end;
//	if(get_selection_range(start, end)){
//		selection_active = false;
//		cursor_position = selection_start;
//	}else{
//		end = 0;
//	}
	
	end = 0;
	if(update_byte){
		buffer->update_byte(key, cursor_nibble/2, start, end);
	}else{
		buffer->update_nibble(key, cursor_nibble, start, end);
	}
	move_cursor_nibble(update_byte ? 2 : 1);
	update_window();
	update_save_state(1);
}

void hex_editor::update_undo_action(bool direction)
{
	if(!is_active){
		return;
	}
	update_save_state((direction << 1) + -1);
	if(get_buffer_position(cursor_position) > buffer->size()){
		cursor_position = get_byte_position(buffer->size());
	}
	set_selection_active(false);
	is_dragging = false;
	update_window();
}

void hex_editor::goto_offset(int address)
{
	if(!buffer->is_active()){
		return;
	}

	address = buffer->snes_to_pc(address);
	offset = address - (rows / 2) * columns;
	offset -= offset % columns;
	if(offset < 0){
		offset = 0;
	}else if(offset > buffer->size() - rows * columns){
		offset = buffer->size() - rows * columns;
	}
	cursor_position = get_byte_position(address);
	set_selection_active(false);
	update_window();
}

void hex_editor::select_range(int start, int end)
{
	if(!buffer->is_active()){
		return;
	}
	start = buffer->snes_to_pc(start);
	end = buffer->snes_to_pc(end);

	offset = start - (rows / 2) * columns;
	offset -= offset % columns;
	if(offset < 0){
		offset = 0;
	}else if(offset > buffer->size() - rows * columns){
		offset = buffer->size() - rows * columns;
	}
	selection_start = get_byte_position(start);
	selection_current = get_byte_position(end);
	set_selection_active(true);
	update_window();
}

void hex_editor::context_menu(const QPoint& position)
{	
	QMenu menu;
	menu.addAction("Cut", this, SLOT(cut()), QKeySequence::Cut)->setEnabled(selection_active);
	menu.addAction("Copy", this, SLOT(copy()), QKeySequence::Copy)->setEnabled(selection_active);
	menu.addAction("Paste", this, SLOT(paste()), QKeySequence::Paste)->setEnabled(buffer->check_paste_data());
	menu.addAction("Delete", this, SLOT(delete_text()), QKeySequence::Delete)->setEnabled(selection_active);
	menu.addSeparator();
	menu.addAction("Select all", this, SLOT(select_all()), QKeySequence::SelectAll);
	menu.addSeparator();
	menu.addAction("Follow branch", this, 
	               SLOT(branch()), QKeySequence("Ctrl+b"))->setEnabled(follow_selection(true));
	menu.addAction("Follow jump", this, 
	               SLOT(jump()), QKeySequence("Ctrl+j"))->setEnabled(follow_selection(false));
	menu.addAction("Disassemble", this, 
	               SLOT(disassemble()), QKeySequence("Ctrl+d"))->setEnabled(selection_active);
	menu.addSeparator();
	menu.addAction("Bookmark", this, 
	               SLOT(create_bookmark()), QKeySequence("Ctrl+b"))->setEnabled(selection_active);
	
	menu.exec(mapToGlobal(position));
}

void hex_editor::cut()
{
	int start, end;
	if(!buffer->is_active() || !get_selection_range(start, end)){
		return;
	}
	
	buffer->cut(start, end, click_side);
	cursor_position = selection_start;
	set_selection_active(false);
	update_window();
	update_save_state(1);
}

void hex_editor::copy()
{
	int start, end;
	if(!buffer->is_active() || !get_selection_range(start, end)){
		return;
	}
	
	buffer->copy(start, end, click_side);
	update_window();
	update_save_state(1);
}

void hex_editor::paste(bool raw)
{
	if(!buffer->is_active()){
		return;
	}
	int start, end;
	if(get_selection_range(start, end)){
		buffer->paste(start, end, raw);
	}else{
		int size = buffer->paste(get_buffer_position(cursor_position), 0, raw);
		cursor_position = get_byte_position(get_buffer_position(cursor_position)+size);
		set_selection_active(false);
	}
	update_window();
	update_save_state(1);
}

void hex_editor::delete_text()
{
	if(!buffer->is_active()){
		return;
	}
	int start, end;
	if(!get_selection_range(start, end)){
		buffer->delete_text(get_buffer_position(cursor_position));
	}else{
		buffer->delete_text(start, end);	
		set_selection_active(false);
		cursor_position = selection_start;
	}
	update_window();
	update_save_state(1);
}

void hex_editor::select_all()
{
	if(!buffer->is_active()){
		return;
	}
	selection_start = get_byte_position(0);
	selection_current = get_byte_position(buffer->size());
	set_selection_active(true);
	emit update_status_text(get_status_text());
	update();
}

void hex_editor::branch()
{
	int start, end;
	if(!buffer->is_active() || !get_selection_range(start, end)){
		return;
	}
	QString text = "selected bytes: ";
	text.append(QString::number(buffer->branch_address(end, 
	            buffer->to_little_endian(buffer->range(start, end))), 16));
	
	goto_offset(buffer->branch_address(end, 
	            buffer->to_little_endian(buffer->range(start, end))));
}

void hex_editor::jump()
{
	int start, end;
	if(!buffer->is_active() || !get_selection_range(start, end)){
		return;
	}
	QString text = "selected bytes: ";
	text.append(QString::number(buffer->jump_address(end, 
	            buffer->to_little_endian(buffer->range(start, end))), 16));
	
	goto_offset(buffer->jump_address(end, 
	            buffer->to_little_endian(buffer->range(start, end))));
}

void hex_editor::disassemble()
{
	int start, end;
	if(!buffer->is_active() || !get_selection_range(start, end)){
		return;
	}
	emit send_disassemble_data(start, end, buffer);
}

void hex_editor::create_bookmark()
{
	int start, end;
	if(!buffer->is_active() || !get_selection_range(start, end)){
		return;
	}
	emit send_bookmark_data(start, end, buffer);
}

void hex_editor::count(QString find, bool mode)
{
	if(!buffer->is_active()){
		return;
	}
	int result = buffer->count(find, mode);
	if(result < 0){
		search_error(result, find);
	}else{
		update_status_text(QString::number(result) + " Results found for " + find);
	}

}

void hex_editor::search(QString find, bool direction, bool mode)
{	
	if(!buffer->is_active()){
		return;
	}
	
	int start, end;
	if(!get_selection_range(start, end)){
		end = get_buffer_position(cursor_position);
	}else if(!direction){
		end = start - 1;
	}
	int result = buffer->search(find, end, direction, mode);
	if(result < 0){
		search_error(result, find);
	}else{
		int start = buffer->pc_to_snes(result);
		int end = 0;
		if(mode){
			end = buffer->pc_to_snes(result + buffer->to_hex(find).length()/2 - 1);
		}else{
			end = buffer->pc_to_snes(result + find.length() - 1);
		}
		goto_offset(end);
		select_range(start, end);
	}
}

void hex_editor::replace(QString find, QString replace, bool direction, bool mode)
{
	if(!buffer->is_active()){
		return;
	}
	int position = get_buffer_position(cursor_position);
	int result = buffer->replace(find, replace, position, direction, mode);
	if(result < 0){
		search_error(result, find, replace);
	}else{
		int start = buffer->pc_to_snes(result);
		int end = 0;
		if(mode){
			end = buffer->pc_to_snes(result + buffer->to_hex(replace).length()/2 - 1);
		}else{
			end = buffer->pc_to_snes(result + replace.length() - 1);
		}
		goto_offset(end);
		select_range(start, end);
	}
	update_save_state(1);
}

void hex_editor::replace_all(QString find, QString replace, bool mode)
{
	if(!buffer->is_active()){
		return;
	}
	int result = buffer->replace_all(find, replace, mode);
	if(result < 0){
		search_error(result, find, replace);
	}else{
		update_status_text(QString::number(result) + " Results found for " + find);
	}
	update_save_state(1);
}

void hex_editor::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);
//	for(int i = hex_offset; i < total_byte_column_width + hex_offset; i += byte_column_width * 2){
//		painter.fillRect(i-1, 0, column_width(2)+2, 
//		                 column_height(rows+1)+vertical_offset, palette().color(QPalette::AlternateBase).darker());
//	}
	
//	if(selection_active){
//		paint_selection(painter);
//	}
	
}

void hex_editor::paint_selection(QPainter &painter)
{		
	int start = get_selection_point(selection_start);
	int end = get_selection_point(selection_current);
	if(start > end){
		qSwap(start, end);
	}
	for(; start < end+1 && end != offset; start++){
		QPoint position = get_byte_position(start);
		painter.fillRect(position.x()-1, position.y(), font_width*3, font_height+4, 
		                 palette().color(QPalette::Active, QPalette::Highlight));
		painter.fillRect(to_ascii_column(position.x()), position.y(), font_width, font_height+4, 
		                 palette().color(QPalette::Active, QPalette::Highlight));
	}
	
	painter.fillRect(column_width(10+columns*3)+1, 0, font_width, column_height(rows+1), 
	                 palette().color(QPalette::Base));
}

void hex_editor::keyPressEvent(QKeyEvent *event)
{
	if(event->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)){
		if(event->key() == Qt::Key_V){
			paste(true);
		}
	}
	
	if(event->modifiers() == Qt::ControlModifier){
		//more hotkeys here if needed
		return;
	}

	//TODO: handle hex vs ascii side
	switch(event->key()){
		case Qt::Key_Backspace:
			move_cursor_nibble((cursor_nibble - 2) & ~1);
			delete_text();
		break;		
		case Qt::Key_Home:
			move_cursor_nibble(-cursor_nibble % (columns * 2));
		break;
		case Qt::Key_End:
			move_cursor_nibble((columns * 2) - (cursor_nibble % (columns * 2)) - 2);
		break;
		case Qt::Key_Up:
			move_cursor_nibble(-columns * 2);
		break;
		case Qt::Key_Down:
			move_cursor_nibble(columns * 2);
		break;
		case Qt::Key_Right:
			move_cursor_nibble(2);
		break;
		case Qt::Key_Left:
			move_cursor_nibble(-2);
		break;
		case Qt::Key_PageUp:
			move_cursor_nibble(-columns * 2 * rows);
		break;
		case Qt::Key_PageDown:
			move_cursor_nibble(columns * 2 * rows);
		break;
		default:
			return;
		break;
	}
	selection_bytes.set_active(false);
	update_window();
}

void hex_editor::wheelEvent(QWheelEvent *event)
{
	int steps = -event->delta() / 8 / 15;
	move_cursor_nibble(columns * 2 * steps);
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
		
		string_stream << "Selection range: $" << buffer->get_formatted_address(position1)
		              << " to $" << buffer->get_formatted_address(position2);
	}else{
		int position = get_buffer_position(cursor_position);
		unsigned char byte = buffer->at(position);
		
		string_stream << "Current offset: $" << buffer->get_formatted_address(position)
		              << "    Hex: 0x" << QString::number(byte, 16).rightJustified(2, '0').toUpper()
		              << "    Dec: " << QString::number(byte).rightJustified(3, '0')
		              << "    Bin: %" << QString::number(byte, 2).rightJustified(8, '0');
	}
	return text;
}

int hex_editor::get_selection_point(QPoint point)
{
	if(point.y() < 0){
		point.setY(vertical_offset);
		point.setX(hex_offset);
	}else if(point.y() > column_height(rows)+vertical_offset - font_height){
		point.setY(column_height(rows)+vertical_offset -font_height);
		point.setX(column_width(11+columns*3)-font_width);
	}
	return get_buffer_position(point);
}

bool hex_editor::get_selection_range(int &start, int &end)
{
	start = get_buffer_position(selection_start);
	end = get_buffer_position(selection_current);
	if(start > end){
		qSwap(start, end);
		qSwap(selection_start, selection_current);
	}
	end++;
	return selection_active;
}

bool hex_editor::follow_selection(bool type)
{
	int start, end;
	if(get_selection_range(start, end)){
		int range = end-start;
		if(type && (range == 1 || range == 2)){
			return true;
		}else if(!type && (range == 2 || range == 3)){
			if(buffer->validate_address(buffer->jump_address(end,
			   buffer->to_little_endian(buffer->range(start, end))), false)){
				return true;                
			}
		}
	}
	return false;
}

int hex_editor::get_buffer_position(QPoint &point, bool byte_align)
{
	return get_buffer_position(point.x(), point.y(), byte_align);
}

int hex_editor::get_buffer_position(int x, int y, bool byte_align)
{
	int position = (x - hex_offset) / font_width;
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

void hex_editor::move_cursor_nibble(int delta)
{
	cursor_nibble += delta;
	if(cursor_nibble < 0){
		cursor_nibble = 0;
	}else if(cursor_nibble >= buffer->size() * 2){
		cursor_nibble = buffer->size() * 2 - 1;  //last nibble
	}
	
	//TODO optimize
	while(cursor_nibble/2 >= offset + rows * columns){
		offset += columns;
	}
	while(cursor_nibble/2 < offset){
		offset -= columns;
	}
	
	update_window();
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
//	int old_offset = offset;
//	QPoint last_byte = get_byte_position(buffer->size()-1);
//	if(x < hex_offset){
//		x = hex_offset;
//	}else if(x >= column_width(10+columns*3)-font_width){
//		x = column_width(10+columns*3)-font_width*2;
//	}else if(x >= last_byte.x()-font_width && y-font_height >= last_byte.y()){
//		x = last_byte.x()-font_width*2;
//	}
//	x -= font_width;
	
//	if(y < vertical_offset && !offset){
//		y = vertical_offset;
//	}else if(y > vertical_offset + column_height(rows) && offset == buffer->size()-rows*columns){
//		y = vertical_offset + column_height(rows);
//	}
	
//	QPoint position = get_byte_position(get_buffer_position(x+font_width, y-vertical_shift));
//	update_cursor_position(position.x(), position.y(), false);
	
//	if(old_offset != offset){
//		selection_start.setY(selection_start.y() - (offset - old_offset));
//	}
//	selection_current = cursor_position;
//	update_window();
}

void hex_editor::update_window()
{
	if(!scroll_mode){
		emit update_slider(offset / columns);
		emit update_range(get_max_lines()+1);
	}else{
		emit update_range(height());
	}
	emit update_status_text(get_status_text());
	ascii->update_display();
	hex->update_display();
	address->update_display();
	update();
	emit selection_toggled(selection_active);
}

void hex_editor::search_error(int error, QString find, QString replace_with)
{
	if(error == ROM_buffer::INVALID_REPLACE){
		update_status_text("Error: Invalid replace hex string: " + replace_with);
	}else if(error == ROM_buffer::INVALID_FIND){
		update_status_text("Error: Invalid find hex string: " + find);
	}else if(error == ROM_buffer::NOT_FOUND){
		update_status_text("Error: String " + find + " not found.");
	}
}

hex_editor::~hex_editor()
{
	emit selection_toggled(false);
	emit clipboard_usable(false);
	emit focused(false);
	delete buffer;
}

const QString hex_editor::offset_header = "Offset     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";
