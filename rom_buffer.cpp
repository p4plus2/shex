#include "rom_buffer.h"
#include "undo_commands.h"
#include "debug.h"
#include "utility.h"
#include "character_mapper.h"

ROM_buffer::ROM_buffer(QString file_name, bool new_file)
{
	if(!new_file){
		open(file_name);
		get_rats_tags();
	}else{
		buffer.fill(0x00, 0x8000);
	}
	clipboard = QApplication::clipboard(); //shared by all, but work around static initialization order
	qDebug() << ENUM_STRING(memory_mapper, get_mapper());
}

void ROM_buffer::remove_copy_header()
{
	header_buffer = buffer.mid(0, header_size());
	buffer.remove(0, header_size());
}

void ROM_buffer::open(QString path)
{
	ROM.setFileName(path);
	ROM.open(QFile::ReadOnly);
	buffer = ROM.readAll();
	if(ROM.size() < 0x8000){
		ROM_error = "The ROM is too small to be valid.";
		return;
	}
	ROM.close();
	analyze();
}

void ROM_buffer::save(QString path)
{
	QFileInfo info(ROM);
	if(path != "" && path != info.absolutePath()){
		ROM.setFileName(path);	
	}
	ROM.open(QFile::WriteOnly);
	if(header_size()){
		ROM.seek(0);
		ROM.write(header_buffer);
	}
	ROM.seek(header_size());
	ROM.write(buffer);
	ROM.close();
}

void ROM_buffer::initialize_undo(QUndoGroup *undo_group)
{
	undo_stack = new QUndoStack(undo_group);
	undo_stack->setActive();
}

void ROM_buffer::cut(int start, int end, bool ascii_mode)
{
	undo_stack->beginMacro("Cut");
	copy(start, end, ascii_mode);
	delete_text(start, end);
	undo_stack->endMacro();
}

void ROM_buffer::copy(int start, int end, bool ascii_mode)
{	
	if(ascii_mode){
		QByteArray text_data = character_mapper::encode(buffer.mid(start, end-start));
		for(int i = 0; i < text_data.length(); i++){
			if(!isprint(character_mapper::encode(text_data.at(i)))){
				text_data[i] = '.';	                
			}
		}
		
		clipboard->setText(text_data);
		return;
	}
	
	QString copy_data = copy_format(start, end, copy_type);
	clipboard->setText(copy_data);
}

QString ROM_buffer::copy_format(int start, int end, copy_style style)
{
	QByteArray hex_data = buffer.mid(start, end-start).toHex().toUpper();
	int nibble_count = hex_data.length();
	QString copy_data;
	QTextStream stream(&copy_data);
	
	switch(style){
		case NO_SPACES:
			copy_data = hex_data;
		break;
		case SPACES:
			for(int i = 0; i < nibble_count; i+=2){
				stream << hex_data[i] << hex_data[i+1] << ' ';
			}
			copy_data.chop(1);
		break;
		case HEX_FORMAT:
			for(int i = 0; i < nibble_count; i+=2){
				stream << '$' << hex_data[i] << hex_data[i+1] << ", ";
			}
			copy_data.chop(2);
		break;
		case ASM_BYTE_TABLE:
			for(int i = 0; i < nibble_count;){
				stream << "db ";
				for(int j = 0; j < 16 && i < nibble_count; j += 2, i += 2){
					stream << '$' << hex_data[i] << hex_data[i+1] << ", ";
				}
				copy_data.chop(2);
				stream << '\n';
			}
			copy_data.chop(1);
		break;
		case ASM_WORD_TABLE:
			for(int i = 0; nibble_count - i >= 4; i += 4){
				stream << "dw $" 
				       << hex_data[i+2] << hex_data[i+3]
				       << hex_data[i]   << hex_data[i+1] << '\n';
			}
			copy_data.chop(1);
		break;
		case ASM_LONG_TABLE:
			for(int i = 0; nibble_count - i >= 6; i += 6){
				stream << "dl $" 
				       << hex_data[i+4] << hex_data[i+5] 
				       << hex_data[i+2] << hex_data[i+3]
				       << hex_data[i]   << hex_data[i+1] << '\n';
			}
			copy_data.chop(1);
		break;
		case C_SOURCE:
			stream << "const unsigned char hex_data[] = {\n";
			for(int i = 0; i < nibble_count;){
				stream << '\t';
				for(int j = 0; j < 24 && i < nibble_count; j+=2, i+=2){
					stream << "0x" << hex_data[i] << hex_data[i+1] << ",";
				}
				copy_data.chop(1);
				stream << ",\n";
			}
			copy_data.chop(2);
			stream << "\n};";
		break;
	}
	return copy_data;
}

int ROM_buffer::paste(int start, int end, bool raw)
{
	if(!check_paste_data()){
		return 0;
	}
	QString copy_data = clipboard->text().toUtf8().trimmed();
	QByteArray hex_data;
	if(!raw){
		if(copy_data.indexOf("const unsigned char") != -1){
			copy_data.remove(0, copy_data.indexOf('{'));
		}else if(copy_data.indexOf("db $") == 0){
			copy_data.remove(0, 3);
		}
	
		copy_data.remove(QRegExp("(0x|[\\t ])"));
		copy_data.remove(QRegExp("([\\n\\r]db|dw|dl|[^0-9A-Fa-f])"));
		hex_data = hex_data.fromHex(copy_data.toUtf8());
	}else{
		hex_data = hex_data.fromHex(copy_data.toUtf8().toHex());
	}
	undo_stack->beginMacro("Paste");
	if(end){
		delete_text(start, end);
	}
	buffer.insert(start, hex_data);
	undo_stack->push(new undo_paste_command(&buffer, start, new QByteArray(hex_data)));
	undo_stack->endMacro();
	return hex_data.size();
}

void ROM_buffer::delete_text(int start, int end)
{
	if(!end){
		end = start + 1;
	}
	undo_stack->beginMacro("Delete");
	QByteArray data(buffer.mid(start, end-start));
	undo_stack->push(new undo_delete_command(&buffer, start, new QByteArray(data)));
	undo_stack->endMacro();
	buffer.remove(start, end-start);
}

void ROM_buffer::update_nibble(char byte, int position, int delete_start, int delete_end)
{
	bool remove = false;
	if(position/2 == buffer.size()){
		buffer[position/2] = 0;
		remove = true;
	}
	undo_stack->beginMacro("Typing");
	if(delete_end){
		delete_text(delete_start, delete_end);
	}
	
	unsigned char data[2] = {(unsigned char)buffer[position/2], 0};
	buffer[position/2] = (buffer.at(position/2) &
			     ((0x0F >> ((position & 1) << 2)) | (0x0F << ((position & 1) << 2)))) |
			     (byte << (((position & 1)^1) << 2));
	data[1] = buffer[position/2];
	undo_stack->push(new undo_nibble_command(&buffer, position/2, data, remove));
	undo_stack->endMacro();
}

void ROM_buffer::update_byte(char byte, int position, int delete_start, int delete_end)
{
	bool remove = false;
	if(position == buffer.size()){
		buffer[position] = 0;
		remove = true;
	}
	undo_stack->beginMacro("Typing");
	if(delete_end){
		delete_text(delete_start, delete_end);
	}
	unsigned char data[2] = {(unsigned char)buffer[position],(unsigned char)byte};
	undo_stack->push(new undo_byte_command(&buffer, position, data, remove));
	buffer[position] = byte;
	undo_stack->endMacro();
}

QString ROM_buffer::get_formatted_address(int address) const
{
	address = pc_to_snes(address);
	int bank = address >> 16;
	int word = address & 0xFFFF;
	if(address < 0){
		return QString("NOT:ROM");
	}
	return '$' + to_hex(bank) + ':' + to_hex(word, 4);
}


int ROM_buffer::count(QString find, bool mode)
{
	QByteArray search_for = input_to_byte_array(find, mode);
	if(search_for.isEmpty()){
		return INVALID_FIND;
	}
	return buffer.count(search_for);
}

int ROM_buffer::search(QString find, int position, bool direction, bool mode)
{	
	position /= 2;
	QByteArray search_for = input_to_byte_array(find, mode);
	if(search_for.isEmpty()){
		return INVALID_FIND;
	}
	int found_at = NOT_FOUND;
	for(int pass = 0; pass < 2 && found_at == NOT_FOUND; pass++){
		if(direction){
			found_at = buffer.indexOf(search_for, position);
			position = 0;
		}else{
			found_at = buffer.lastIndexOf(search_for, position);
			position = -1;
		}
	}
	return found_at;
}

int ROM_buffer::replace(QString find, QString replace, int position, bool direction, bool mode)
{
	position /= 2;
	int result = search(find, position, direction, mode);
	if(result < 0){
		return result;
	}
	QByteArray replace_with = input_to_byte_array(replace, mode);
	if(replace_with.isEmpty() && !replace.isEmpty()){
		return INVALID_REPLACE;
	}
	undo_stack->beginMacro("Replace");
	delete_text(result, result+input_to_byte_array(find, mode).length());
	undo_stack->push(new undo_paste_command(&buffer, result, new QByteArray(replace_with)));
	buffer.insert(result, replace_with);
	undo_stack->endMacro();
	return result;
}

int ROM_buffer::replace_all(QString find, QString replace, bool mode)
{
	QByteArray search_for = input_to_byte_array(find, mode);
	QByteArray replace_with = input_to_byte_array(replace, mode);
	if(search_for.isEmpty()){
		return INVALID_FIND;
	}else if(replace_with.isEmpty() && !replace.isEmpty()){
		return INVALID_REPLACE;
	}
	int results = count(find, mode);
	undo_stack->beginMacro("Replace All");
	int next = 0;
	for(int i = 0; i < results; i++){
		next = buffer.indexOf(search_for, next);;
		delete_text(next, next+search_for.length());
		undo_stack->push(new undo_paste_command(&buffer, next, new QByteArray(replace_with)));
		buffer.insert(next, replace_with);
	}
	undo_stack->endMacro();
	return results;
}

QVector<int> ROM_buffer::get_rats_tags() const
{
	QVector<int> offsets;
	
	int position = 0;
	do{
		position = buffer.indexOf("STAR", position);
		if((read_word(buffer, position+4) ^ read_word(buffer, position+6)) == 0xFFFF){
			offsets.append(position);
		}
		position++;
	}while(position > 0);
	
	return offsets;
}

QByteArray ROM_buffer::input_to_byte_array(QString input, int mode)
{
	if(mode){
		QString hex = get_hex(input);
		if(hex.length() & 1){
			return QByteArray();
		}else{
			return QByteArray::fromHex(hex.toUtf8());
		}
	}
	return character_mapper::decode(input.toUtf8());
}

ROM_buffer::copy_style ROM_buffer::copy_type = ROM_buffer::NO_SPACES;
QClipboard *ROM_buffer::clipboard;
