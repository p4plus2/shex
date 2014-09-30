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
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(context_menu(const QPoint&)));
	connect(qApp->clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboard_changed()));
	
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
	emit selection_toggled(selection_area.is_active());
	emit focused(true);
	emit update_save_state(0);
	clipboard_changed();
}

void hex_editor::slider_update(int position)
{
	//TODO check this
	if(!scroll_mode){
		move_cursor_nibble(offset - position * text_display::get_columns());
		offset = position * text_display::get_columns();
	}else{
		position -= height() / 2;
		if(position < 0){
	//		scroll_direction = false;
			position = -position;
		}else if(position > 0){
	//		scroll_direction = true;
		}else{
	//		scroll_speed = INT_MAX;
	//		scroll_timer->setInterval(scroll_speed);
			return;
		}
	//	scroll_speed = qAbs(((position - (height() /2))-1) / 15);
	//	scroll_timer->setInterval(scroll_speed);
	}
}

void hex_editor::scroll_mode_changed()
{
	scroll_mode = !scroll_mode;
	update_window();
	emit toggle_scroll_mode(scroll_mode);
}
void hex_editor::control_auto_scroll(bool enabled)
{
	//auto_scrolling = enabled;
	//if(auto_scrolling){
	//	scroll_timer->start(scroll_speed);
	//}else{
	//	scroll_timer->stop();
	//}
}

void hex_editor::handle_typed_character(unsigned char key, bool update_byte)
{
	int start = selection_area.get_start();
	int end = selection_area.get_end();
	if(!selection_area.is_active()){
		end = 0;
	}
	
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
	if(cursor_nibble / 2 > buffer->size()){
		cursor_nibble = buffer->size() * 2 - 1;
	}
	selection_area.set_active(false);
	update_window();
}

void hex_editor::goto_offset(int address)
{
	if(!buffer->is_active()){
		return;
	}

	address = buffer->snes_to_pc(address);
	offset = address - (text_display::get_rows() / 2) * text_display::get_columns();
	offset -= offset % text_display::get_columns();
	if(offset < 0){
		offset = 0;
	}else if(offset > buffer->size() - text_display::get_rows() * text_display::get_columns()){
		offset = buffer->size() - text_display::get_rows() * text_display::get_columns();
	}
	cursor_nibble = address * 2;
	selection_area.set_active(false);
	update_window();
}

void hex_editor::select_range(int start, int end)
{
	if(!buffer->is_active()){
		return;
	}
	start = buffer->snes_to_pc(start);
	end = buffer->snes_to_pc(end);

	offset = start - (text_display::get_rows() / 2) * text_display::get_columns();
	offset -= offset % text_display::get_columns();
	if(offset < 0){
		offset = 0;
	}else if(offset > buffer->size() - text_display::get_rows() * text_display::get_columns()){
		offset = buffer->size() - text_display::get_rows() * text_display::get_columns();
	}
	selection_area.set_start(start * 2);
	selection_area.set_end(end * 2);
	selection_area.set_active(true);
	update_window();
}

void hex_editor::context_menu(const QPoint& position)
{	
	QMenu menu;
	menu.addAction("Cut", this, SLOT(cut()), QKeySequence::Cut)->setEnabled(selection_area.is_active());
	menu.addAction("Copy", this, SLOT(copy()), QKeySequence::Copy)->setEnabled(selection_area.is_active());
	menu.addAction("Paste", this, SLOT(paste()), QKeySequence::Paste)->setEnabled(buffer->check_paste_data());
	menu.addAction("Delete", this, SLOT(delete_text()), 
	               QKeySequence::Delete)->setEnabled(selection_area.is_active());
	menu.addSeparator();
	menu.addAction("Select all", this, SLOT(select_all()), QKeySequence::SelectAll);
	menu.addSeparator();
	menu.addAction("Follow branch", this, 
	               SLOT(branch()), QKeySequence("Ctrl+b"))->setEnabled(follow_selection(true));
	menu.addAction("Follow jump", this, 
	               SLOT(jump()), QKeySequence("Ctrl+j"))->setEnabled(follow_selection(false));
	menu.addAction("Disassemble", this, 
	               SLOT(disassemble()), QKeySequence("Ctrl+d"))->setEnabled(selection_area.is_active());
	menu.addSeparator();
	menu.addAction("Bookmark", this, 
	               SLOT(create_bookmark()), QKeySequence("Ctrl+b"))->setEnabled(selection_area.is_active());
	
	menu.exec(mapToGlobal(position));
}

void hex_editor::cut()
{
	if(!buffer->is_active() || !selection_area.is_active()){
		return;
	}
	
	buffer->cut(selection_area.get_start(), selection_area.get_end(), ascii->hasFocus());
	cursor_nibble = selection_area.get_start();
	selection_area.set_active(false);
	update_window();
	update_save_state(1);
}

void hex_editor::copy()
{
	if(!buffer->is_active() || !selection_area.is_active()){
		return;
	}
	
	buffer->copy(selection_area.get_start(), selection_area.get_end(), ascii->hasFocus());
	update_window();
}

void hex_editor::paste(bool raw)
{
	if(!buffer->is_active()){
		return;
	}
	if(!selection_area.is_active()){
		buffer->paste(selection_area.get_start(), selection_area.get_end(), raw);
	}else{
		int size = buffer->paste(cursor_nibble, 0, raw);
		cursor_nibble = cursor_nibble + size;
		selection_area.set_active(false);
	}
	update_window();
	update_save_state(1);
}

void hex_editor::delete_text()
{
	if(!buffer->is_active()){
		return;
	}
	if(!selection_area.is_active()){
		buffer->delete_text(cursor_nibble);
	}else{
		buffer->delete_text(selection_area.get_start(), selection_area.get_end());	
		selection_area.set_active(false);
		cursor_nibble = selection_area.get_start();
	}
	update_window();
	update_save_state(1);
}

void hex_editor::select_all()
{
	if(!buffer->is_active()){
		return;
	}
	selection_area.set_start(0);
	selection_area.set_end(buffer->size()*2);
	selection_area.set_active(true);
	emit update_status_text(get_status_text());
	update_window();
}

void hex_editor::branch()
{
	if(!buffer->is_active() || !selection_area.is_active()){
		return;
	}

	goto_offset(buffer->branch_address(selection_area.get_end(), 
	            buffer->to_little_endian(buffer->range(selection_area.get_start(), selection_area.get_end()))));
}

void hex_editor::jump()
{
	if(!buffer->is_active() || !selection_area.is_active()){
		return;
	}
	
	goto_offset(buffer->jump_address(selection_area.get_end(), 
	            buffer->to_little_endian(buffer->range(selection_area.get_start(), selection_area.get_end()))));
}

void hex_editor::disassemble()
{
	if(!buffer->is_active() || !selection_area.is_active()){
		return;
	}
	emit send_disassemble_data(selection_area.get_start(), selection_area.get_end(), buffer);
}

void hex_editor::create_bookmark()
{
	if(!buffer->is_active() || !selection_area.is_active()){
		return;
	}
	emit send_bookmark_data(selection_area.get_start(), selection_area.get_end(), buffer);
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
	int end = selection_area.get_end();
	if(!selection_area.is_active()){
		end = cursor_nibble;
	}else if(!direction){
		end = selection_area.get_start() - 1;
	}
	int result = buffer->search(find, end, direction, mode);
	if(result < 0){
		search_error(result, find);
	}else{
		handle_search_result(find, result, mode);
	}
}

void hex_editor::replace(QString find, QString replace, bool direction, bool mode)
{
	if(!buffer->is_active()){
		return;
	}
	int position = cursor_nibble;
	int result = buffer->replace(find, replace, position, direction, mode);
	if(result < 0){
		search_error(result, find, replace);
	}else{
		handle_search_result(replace, result, mode);
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

	switch(event->key()){
		case Qt::Key_Backspace:
			move_cursor_nibble((cursor_nibble - 2) & ~1);
			delete_text();
		break;		
		case Qt::Key_Home:
			move_cursor_nibble(-cursor_nibble % (text_display::get_columns() * 2));
		break;
		case Qt::Key_End:
			move_cursor_nibble((text_display::get_columns() * 2) - 
			                   (cursor_nibble % (text_display::get_columns() * 2)) - 2);
		break;
		case Qt::Key_Up:
			move_cursor_nibble(-text_display::get_columns() * 2);
		break;
		case Qt::Key_Down:
			move_cursor_nibble(text_display::get_columns() * 2);
		break;
		case Qt::Key_Right:
			move_cursor_nibble(1 + ascii->hasFocus());
		break;
		case Qt::Key_Left:
			move_cursor_nibble(-1 - ascii->hasFocus());
		break;
		case Qt::Key_PageUp:
			move_cursor_nibble(-text_display::get_columns() * 2 * text_display::get_rows());
		break;
		case Qt::Key_PageDown:
			move_cursor_nibble(text_display::get_columns() * 2 * text_display::get_rows());
		break;
		default:
			return;
		break;
	}
	selection_area.set_active(false);
	update_window();
}

void hex_editor::wheelEvent(QWheelEvent *event)
{
	int steps = -event->delta() / 8 / 15;
	move_cursor_nibble(text_display::get_columns() * 2 * steps);
}

void hex_editor::handle_search_result(QString target, int result, bool mode)
{
	int start = buffer->pc_to_snes(result);
	if(mode){
		result += buffer->to_hex(target).length()/2 - 1;
	}else{
		result += target.length() - 1;
	}
	goto_offset(buffer->pc_to_snes(result)); //offsets are byte aligned
	select_range(start, buffer->pc_to_snes(result));
}

QString hex_editor::get_status_text()
{
	QString text;
	QTextStream string_stream(&text);
	if(selection_area.is_active()){
		string_stream << "Selection range: $" << buffer->get_formatted_address(selection_area.get_start() / 2)
		              << " to $" << buffer->get_formatted_address(selection_area.get_end() / 2);
	}else{
		int position = cursor_nibble / 2;
		unsigned char byte = buffer->at(position);
		
		string_stream << "Current offset: $" << buffer->get_formatted_address(position)
		              << "    Hex: 0x" << QString::number(byte, 16).rightJustified(2, '0').toUpper()
		              << "    Dec: " << QString::number(byte).rightJustified(3, '0')
		              << "    Bin: %" << QString::number(byte, 2).rightJustified(8, '0');
	}
	return text;
}

bool hex_editor::follow_selection(bool type)
{
	if(selection_area.is_active()){
		int range = selection_area.range();
		if(type && (range == 1 || range == 2)){
			return true;
		}else if(!type && (range == 2 || range == 3)){
			if(buffer->validate_address(buffer->jump_address(selection_area.get_end(),
			   buffer->to_little_endian(buffer->range(selection_area.get_start(), 
			   selection_area.get_end()))), false)){
				return true;                
			}
		}
	}
	return false;
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
	while(cursor_nibble/2 >= offset + text_display::get_rows() * text_display::get_columns()){
		offset += text_display::get_columns();
	}
	while(cursor_nibble/2 < offset){
		offset -= text_display::get_columns();
	}
	
	update_window();
}

void hex_editor::update_window()
{
	if(!scroll_mode){
		emit update_slider(offset / text_display::get_columns());
		emit update_range(get_max_lines()+1);
	}else{
		emit update_range(height());
	}
	emit update_status_text(get_status_text());
	ascii->update_display();
	hex->update_display();
	address->update_display();
	emit selection_toggled(selection_area.is_active());
}

void hex_editor::set_offset(int o)
{
	if(o < 0){
		o = 0;
	}else if(o > buffer->size()){
		o = buffer->size();
	}

	offset = o;
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

int hex_editor::get_max_lines()
{
	return buffer->size() / text_display::get_columns() - text_display::get_rows();
}

hex_editor::~hex_editor()
{
	emit selection_toggled(false);
	emit clipboard_usable(false);
	emit focused(false);
	delete buffer;
}

const QString hex_editor::offset_header = "Offset     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F";
