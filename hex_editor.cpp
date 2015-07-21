#include <QMenu>
#include <QMessageBox>

#include "hex_editor.h"
#include "character_mapper.h"
#include "displays/hex_display.h"
#include "displays/ascii_display.h"
#include "displays/address_display.h"
#include "debug.h"
#include "utility.h"
#include "settings_manager.h"

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
	connect(this, &hex_editor::customContextMenuRequested, this, &hex_editor::context_menu);
	
	address = new address_display(buffer, this);
	hex = new hex_display(buffer, this);
	ascii = new ascii_display(buffer, this);
	
	compare_address = new address_display(compare_buffer, this);
	compare_hex = new hex_display(compare_buffer, this);
	compare_ascii = new ascii_display(compare_buffer, this);
	
	connect(hex, &hex_display::character_typed, this, &hex_editor::handle_typed_character);
	connect(ascii, &ascii_display::character_typed, this, &hex_editor::handle_typed_character);
	
	address_header->setFont(editor_font::get_font());
	hex_header->setFont(editor_font::get_font());
	
	QGridLayout *layout = new QGridLayout();
	layout->addWidget(address_header, 0, 0, Qt::AlignBottom);
	layout->addWidget(hex_header, 0, 1, Qt::AlignBottom);
	
	layout->addWidget(address, 1, 0);
	layout->addWidget(hex, 1, 1);
	layout->addWidget(ascii, 1, 2);
	
	layout->addWidget(compare_address, 2, 0);
	layout->addWidget(compare_hex, 2, 1);
	layout->addWidget(compare_ascii, 2, 2);
	
	layout->setRowStretch(1, 1);
	setLayout(layout);
	
	settings_manager::add_listener(this, {"editor/wheel_cursor",
	                                      "editor/size_change"
	                               });
	settings_manager::add_persistent_listener(this, "display/font");
}

void hex_editor::set_focus()
{
	emit update_status_text(get_status_text());
	hex->setFocus();
	buffer->set_active();
	emit update_save_state(0);
	update_window();
}

void hex_editor::compare(QString file)
{
	comparing = true;
	compare_buffer->open(file);
	QGridLayout *grid = (QGridLayout *)layout();
	
	//work around this eventually... but it works for now
	address->setMaximumHeight(address->size().height() / 2 + 1);
	hex->setMaximumHeight(hex->size().height() / 2 + 1);
	ascii->setMaximumHeight(ascii->size().height() / 2 + 1);
	address->setMaximumHeight(QWIDGETSIZE_MAX);
	hex->setMaximumHeight(QWIDGETSIZE_MAX);
	ascii->setMaximumHeight(QWIDGETSIZE_MAX);
	
	compare_address->show();
	compare_ascii->show();
	compare_hex->show();
	
	grid->setRowStretch(2, 1);
	calculate_diff();
}

void hex_editor::close_compare()
{
	comparing = false;
	QGridLayout *grid = (QGridLayout *)layout();
	
	compare_address->hide();
	compare_ascii->hide();
	compare_hex->hide();
	
	grid->setRowStretch(2, 0);
}

void hex_editor::goto_diff(bool direction)
{
	if(!comparing){
		return;
	}
	int offset = -1;
	for(auto &diff : *diffs){
		if(direction && diff.get_start() > get_cursor_nibble()){
			offset = diff.get_start_byte();
			break;
		}else if(!direction){
			if(diff.get_start() < get_cursor_nibble()){
				offset = diff.get_start_byte();
			}else{
				break;
			}
		}
	}
	if(offset == -1 && diffs->size()){
		if(direction){
			offset = diffs->first().get_start_byte();
		}else{
			offset = diffs->last().get_start_byte();
		}
	}

	if(offset != -1){
		goto_offset(buffer->pc_to_snes(offset));
	}else{
		update_status_text("No differences found!");
	}
}

QString hex_editor::generate_patch()
{
	QString patch;
	QTextStream stream(&patch);
	for(auto &diff : *diffs){
		int start = diff.get_start_byte();
		stream << "ORG $" << to_hex(buffer->pc_to_snes(start), 6)
		       << "\n"    << buffer->copy_format(start, diff.get_end_byte(), ROM_buffer::ASM_BYTE_TABLE) 
		       << "\n\n";
	}
	
	return patch.replace("\ndb", "\n\tdb");
}

bool hex_editor::follow_selection(bool type)
{
	if(selection_area.is_active()){
		int range = selection_area.byte_range();
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

void hex_editor::slider_update(int position)
{
	if(!scroll_mode){
		offset = position * text_display::get_columns();
	}else{
		hex->set_auto_scroll_speed((position - height() / 2) / 10);
	}
	update_window();
}

void hex_editor::scroll_mode_changed()
{
	scroll_mode = !scroll_mode;
	update_window();
	emit toggle_scroll_mode(scroll_mode);
}

void hex_editor::update_window()
{
	if(!scroll_mode){
		emit update_range(get_max_lines()+1);
		emit update_slider(offset / text_display::get_columns());
	}else{
		emit update_range(height());
	}
	emit update_status_text(get_status_text());
	ascii->update_display();
	hex->update_display();
	address->update_display();
	
	if(comparing){
		compare_ascii->update_display();
		compare_hex->update_display();
		compare_address->update_display();
	}
}

void hex_editor::handle_typed_character(unsigned char key, bool update_byte)
{
	if(selection_area.is_active() && !validate_resize()){
		return;
	}
	int start = selection_area.get_start();
	int end = selection_area.get_end();
	if(!selection_area.is_active()){
		end = 0;
	}
	
	if(update_byte){
		buffer->update_byte(key, cursor_nibble / 2, start, end);
	}else{
		buffer->update_nibble(key, cursor_nibble, start, end);
	}
	if(selection_area.is_active()){
		selection_area.set_active(false);
		cursor_nibble = selection_area.get_start();
		move_cursor_nibble(0);
	}else{
		move_cursor_nibble(update_byte ? 2 : 1);
	}
	update_save_state(1);
}

void hex_editor::update_undo_action(bool direction)
{
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
	
	if(!buffer->validate_address(address, false)){
		emit update_status_text(buffer->get_address_error());
		return;
	}

	address = buffer->snes_to_pc(address);
	offset = address - text_display::get_rows_by_columns() / 2;
	offset -= offset % text_display::get_columns();
	offset = clamp(offset, 0, buffer->size() - text_display::get_rows_by_columns());
	
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

	offset = start - text_display::get_rows_by_columns() / 2;
	offset -= offset % text_display::get_columns();
	offset = clamp(offset, 0, buffer->size() - text_display::get_rows_by_columns());
	
	selection_area.set_start(start * 2);
	selection_area.set_end(end * 2 + 1);
	update_window();
}

void hex_editor::context_menu(const QPoint& position)
{	
	QMenu menu;
	typedef QKeySequence hotkey;
	bool selection_active = selection_area.is_active();
	menu.addAction("Cut", this, SLOT(cut()), hotkey::Cut)->setEnabled(selection_active);
	menu.addAction("Copy", this, SLOT(copy()), hotkey::Copy)->setEnabled(selection_active);
	menu.addAction("Paste", this, SLOT(paste()), hotkey::Paste)->setEnabled(buffer->check_paste_data());
	menu.addAction("Delete", this, SLOT(delete_text()), hotkey::Delete)->setEnabled(selection_active);
	menu.addSeparator();
	menu.addAction("Select all", this, SLOT(select_all()), hotkey::SelectAll);
	menu.addSeparator();
	menu.addAction("Follow branch", this, SLOT(branch()), hotkey("Alt+j"))->setEnabled(follow_selection(true));
	menu.addAction("Follow jump", this, SLOT(jump()), hotkey("Ctrl+j"))->setEnabled(follow_selection(false));
	menu.addAction("Disassemble", this, SLOT(disassemble()), hotkey("Ctrl+d"))->setEnabled(selection_active);
	menu.addSeparator();
	menu.addAction("Bookmark", this, SLOT(create_bookmark()), hotkey("Ctrl+b"))->setEnabled(selection_active);
	menu.exec(mapToGlobal(position));
}

void hex_editor::cut()
{
	if(!selection_area.is_active() || !validate_resize()){
		return;
	}
	
	buffer->cut(selection_area.get_start_byte(), selection_area.get_end_byte(), ascii->hasFocus());
	cursor_nibble = selection_area.get_start_byte();
	selection_area.set_active(false);
	update_window();
	update_save_state(1);
}

void hex_editor::copy()
{
	if(!selection_area.is_active()){
		return;
	}
	
	buffer->copy(selection_area.get_start_byte(), selection_area.get_end_byte(), ascii->hasFocus());
	update_window();
}

void hex_editor::paste(bool raw)
{
	if(!validate_resize()){
		return;
	}
	if(!selection_area.is_active()){
		buffer->paste(selection_area.get_start_byte(), selection_area.get_end_byte(), raw);
	}else{
		int size = buffer->paste(cursor_nibble / 2, 0, raw);
		cursor_nibble = cursor_nibble + size;
		selection_area.set_active(false);
	}
	update_window();
	update_save_state(1);
}

void hex_editor::delete_text()
{
	if(!validate_resize()){
		return;
	}
	if(!selection_area.is_active()){
		move_cursor_nibble((cursor_nibble - 2) & ~1);
		buffer->delete_text(cursor_nibble);
	}else{
		buffer->delete_text(selection_area.get_start_byte(), selection_area.get_end_byte());	
		selection_area.set_active(false);
		cursor_nibble = selection_area.get_start();
	}
	update_window();
	update_save_state(1);
}

void hex_editor::select_all()
{
	selection_area.set_start(0);
	selection_area.set_end(buffer->size()*2);
	emit update_status_text(get_status_text());
	update_window();
}

void hex_editor::branch()
{
	if(!selection_area.is_active()){
		return;
	}

	goto_offset(buffer->branch_address(selection_area.get_end(), 
	            buffer->to_little_endian(buffer->range(selection_area.get_start(), selection_area.get_end()))));
}

void hex_editor::jump()
{
	if(!selection_area.is_active()){
		return;
	}
	
	goto_offset(buffer->jump_address(selection_area.get_end(), 
	            buffer->to_little_endian(buffer->range(selection_area.get_start(), selection_area.get_end()))));
}

void hex_editor::disassemble()
{
	if(!selection_area.is_active()){
		return;
	}
	emit send_disassemble_data(selection_area, buffer);
}

void hex_editor::create_bookmark()
{
	if(!selection_area.is_active()){
		return;
	}
	emit send_bookmark_data(selection_area.get_start_byte(), selection_area.get_end_byte(), buffer);
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
			move_cursor_nibble(-text_display::get_rows_by_columns() * 2);
		break;
		case Qt::Key_PageDown:
			move_cursor_nibble(text_display::get_rows_by_columns() * 2);
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
	if(wheel_cursor){
		move_cursor_nibble(text_display::get_columns() * 2 * steps);
	}else{
		set_offset(offset + text_display::get_columns() * 2 * steps);
	}
	update_window();
}

bool hex_editor::event(QEvent *event)
{
	if(event->type() == (QEvent::Type)SETTINGS_EVENT){
		settings_event *e = (settings_event *)event;
		if(e->data().first == "display/font"){
			address_header->setFont(editor_font::get_font());
			hex_header->setFont(editor_font::get_font());
		}else if(e->data().first == "editor/wheel_cursor"){
			wheel_cursor = e->data().second.toBool();
		}else if(e->data().first == "editor/size_change"){
			prompt_resize = e->data().second.toBool();
		}
	}
	if(event->type() != (QEvent::Type)EDITOR_EVENT){
		return QWidget::event(event);
	}
	editor_events type = ((editor_event *)event)->sub_type();
	switch(type){
		case editor_events::CUT:
			cut();
			return true;
		case editor_events::COPY:
			copy();
			return true;
		case editor_events::PASTE:
			paste();
			return true;
		case editor_events::DELETE_TEXT:
			delete_text();
			return true;
		case editor_events::NO_SPACES ... editor_events::C_SOURCE:
			ROM_buffer::set_copy_style((ROM_buffer::copy_style)(type-editor_events::NO_SPACES));
			return true;
		case editor_events::UNDO:
			update_undo_action(false);
			return true;
		case editor_events::REDO:
			update_undo_action(true);
			return true;
		case editor_events::BRANCH:
			branch();
			return true;
		case editor_events::JUMP:
			jump();
			return true;
		case editor_events::SELECT_ALL:
			select_all();
			return true;
		case editor_events::DISASSEMBLE:
			disassemble();
			return true;
		case editor_events::BOOKMARK:
			create_bookmark();
			return true;
		case editor_events::SCROLL_MODE:
			scroll_mode_changed();
			return true;
		case editor_events::CLOSE_COMPARE:
			close_compare();
			return true;
		case editor_events::PREVIOUS:
			goto_diff(false);
			return true;
		case editor_events::NEXT:
			goto_diff(true);
			return true;
		default:
			qDebug() << "Bad event" << type;
			return false;
	}
}

void hex_editor::handle_search_result(QString target, int result, bool mode)
{
	int start = buffer->pc_to_snes(result);
	if(mode){
		result += buffer->get_hex(target).length()/2;
	}else{
		result += target.length();
	}
	goto_offset(buffer->pc_to_snes(result)); //offsets are byte aligned
	select_range(start, buffer->pc_to_snes(result));
}

void hex_editor::calculate_diff()
{
	if(!diffs){
		diffs = new QVector<selection>;
	}
	diffs->clear();
	
	int start = 0;
	for(int i = 0; i < buffer->size() && i < compare_buffer->size(); i++){
		if(buffer->at(i) != compare_buffer->at(i)){
			continue;
		}
		if(start != i){
			diffs->append(selection::create_selection(start, i - start));
			start = i;
		}
		start++;
	}
}

void hex_editor::update_save_state(int direction)
{
	save_state += direction;
	emit save_state_changed(!save_state);
	if(comparing){
		calculate_diff();
	}
}

QString hex_editor::get_status_text()
{
	QString text;
	QTextStream string_stream(&text);
	if(selection_area.is_active()){
		string_stream << "Selection range: " << buffer->get_formatted_address(selection_area.get_start_byte())
		              << " to " << buffer->get_formatted_address(selection_area.get_end_byte() - 1);
	}else{
		int position = cursor_nibble / 2;
		unsigned char byte = buffer->at(position);
		
		string_stream << "Current offset: " << buffer->get_formatted_address(position)
		              << "    Hex: 0x" << to_hex(byte)
		              << "    Dec: " << QString::number(byte).rightJustified(3, '0')
		              << "    Bin: %" << QString::number(byte, 2).rightJustified(8, '0');
	}
	return text;
}

void hex_editor::move_cursor_nibble(int delta)
{
	cursor_nibble += delta;
	cursor_nibble = clamp(cursor_nibble, 0, buffer->size() * 2 - 1);
	
	//TODO optimize
	while(cursor_nibble/2 >= offset + text_display::get_rows_by_columns()){
		set_offset(offset + text_display::get_columns());
	}
	while(cursor_nibble/2 < offset){
		set_offset(offset -text_display::get_columns());
	}

	update_window();
}

void hex_editor::set_offset(int o)
{
	offset = clamp(o, 0, buffer->size() - text_display::get_rows_by_columns());
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
	return buffer->size() / text_display::get_columns() - text_display::get_rows() - 1;
}

bool hex_editor::validate_resize()
{
	if(prompt_resize){
		return true;
	}
	QString prompt = "Changing the size of the ROM may render it unusable! Do you wish to continue?";
	QMessageBox::StandardButton answer = QMessageBox::question(this, "Warning!", prompt);
	return answer == QMessageBox::Yes;
}

hex_editor::~hex_editor()
{
	delete buffer;
	delete compare_buffer;
	delete diffs;
}

bool hex_editor::wheel_cursor;
bool hex_editor::prompt_resize;
