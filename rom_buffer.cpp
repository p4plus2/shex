#include "rom_buffer.h"
#include "undo_commands.h"

#include <QTextStream>
#include <QRegExp>
#include "QDebug"

ROM_buffer::ROM_buffer(QString file_name)
{
	if(file_name != ""){
		ROM.setFileName(file_name);
		ROM.open(QFile::ReadWrite);
		buffer = ROM.readAll();
	}else{
		buffer[0] = 0;
	}
	clipboard = QApplication::clipboard();
	paste_type = C_SOURCE;
}

void ROM_buffer::initialize_undo(QUndoGroup *undo_group)
{
	undo_stack = new QUndoStack(undo_group);
	undo_stack->setActive();
}

void ROM_buffer::cut(int start, int end)
{
	undo_stack->beginMacro("Cut");
	copy(start, end);
	delete_text(start, end);
	undo_stack->endMacro();
}

void ROM_buffer::copy(int start, int end)
{	
	QByteArray hex_data = buffer.mid(start, end-start).toHex().toUpper();
	QString copy_data;
	QTextStream stream(&copy_data);
	int nibble_count = hex_data.length();
	
	switch(paste_type){
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
				for(int j = 0; j < 16 && i < nibble_count; j+=2, i+=2){
					stream << '$' << hex_data[i] << hex_data[i+1] << ',';
				}
				copy_data.chop(1);
				stream << '\n';
			}
			copy_data.chop(1);
		break;
		case ASM_WORD_TABLE:
			for(int i = 0; i < nibble_count; i+=4){
				stream << "dw $" << hex_data[i] << hex_data[i+1] 
				       << hex_data[i+2] << hex_data[i+3] << '\n';
			}
			copy_data.chop(1);
		break;
		case ASM_LONG_TABLE:
			for(int i = 0; i < nibble_count; i+=6){
				stream << "dl $" << hex_data[i] << hex_data[i+1] 
				       << hex_data[i+2] << hex_data[i+3]
				       << hex_data[i+4] << hex_data[i+5] << '\n';
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
	clipboard->setText(copy_data);
}

int ROM_buffer::paste(int start, int end, bool raw)
{
	if(check_paste_data()){
		return 0;
	}
	QString copy_data = clipboard->text().toUtf8().trimmed();
	if(!raw){
		if(copy_data.indexOf("const unsigned char") != -1){
			copy_data.remove(0, copy_data.indexOf('{'));
		}else if(copy_data.indexOf("db $") == 0){
			copy_data.remove(0, 3);
		}
	
		copy_data.remove(QRegExp("(0x|[\\t ])"));
		copy_data.remove(QRegExp("([\\n\\r]db|dw|dl|[^0-9A-Fa-f])"));
	}
	
	QByteArray hex_data = hex_data.fromHex(copy_data.toUtf8());
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
	
	unsigned char data[2] = {buffer[position/2], 0};
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
	unsigned char data[2] = {buffer[position],(unsigned char)byte};
	undo_stack->push(new undo_byte_command(&buffer, position, data, remove));
	buffer[position] = byte;
	undo_stack->endMacro();
}

QString ROM_buffer::get_line(int index, int length)
{
	QString line;
	QTextStream string_stream(&line);
	string_stream << "$" << get_address(index) << ": ";

	int line_length = index+length;

	if(line_length > buffer.size()){
		line_length = buffer.size();
	}

	for(int i = index; i < line_length; i++){
		string_stream << " " 
		              << QString::number((unsigned char)buffer.at(i),16).rightJustified(2, '0').toUpper();
	}
	line = line.leftJustified(10 + length * 3);
	string_stream << "    ";

	for(int i = index; i < line_length; i++){
		if(isprint((unsigned char)buffer.at(i))){
			string_stream << buffer.at(i);
		}else{
			string_stream << ".";
		}
	}
	line = line.leftJustified(14 + length * 4);
	return line;
}

QString ROM_buffer::get_address(int address)
{
	int bank = (address & 0x7f8000) >> 15;
	int word = 0x8000 + (address & 0x7fff);
	return QString::number(bank, 16).rightJustified(2, '0').toUpper() +
			":" + QString::number(word,16).toUpper();
}
