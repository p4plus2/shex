#include "rom_buffer.h"

#include <QTextStream>
#include "QDebug"

ROM_buffer::ROM_buffer()
{
	ROM.setFileName("smw.smc");
	ROM.open(QFile::ReadWrite);
	buffer = ROM.readAll();
	clipboard = QApplication::clipboard();
	paste_type = C_SOURCE;
}

void ROM_buffer::cut(int start, int end)
{
	copy(start, end);
	buffer.remove(start, end-start);
}

void ROM_buffer::copy(int start, int end)
{	
	QByteArray hex_data = buffer.mid(start, end-start).toHex().toUpper();
	QString copy_data;
	QTextStream stream(&copy_data);
	int nibble_count = hex_data.length();
	
	switch(paste_type){
		case NO_SPACES:
			clipboard->setText(copy_data);
		break;
		case SPACES:
			for(int i = 0; i < nibble_count; i+=2){
				stream << hex_data[i] << hex_data[i+1] << ' ';
			}
			copy_data.truncate(copy_data.length()-1);
			clipboard->setText(copy_data);
		break;
		case HEX_FORMAT:
			for(int i = 0; i < nibble_count; i+=2){
				stream << '$' << hex_data[i] << hex_data[i+1] << ", ";
			}
			copy_data.truncate(copy_data.length()-2);
			clipboard->setText(copy_data);
		break;
		case ASM_BYTE_TABLE:
			for(int i = 0; i < nibble_count;){
				stream << "db ";
				for(int j = 0; j < 16 && i < nibble_count; j+=2, i+=2){
					stream << '$' << hex_data[i] << hex_data[i+1] << ',';
				}
				copy_data.truncate(copy_data.length()-1);
				stream << '\n';
			}
			copy_data.truncate(copy_data.length()-1);
			clipboard->setText(copy_data);
		break;
		case ASM_WORD_TABLE:
			for(int i = 0; i < nibble_count; i+=4){
				stream << "dw $" << hex_data[i] << hex_data[i+1] 
				       << hex_data[i+2] << hex_data[i+3] << '\n';
			}
			copy_data.truncate(copy_data.length()-1);
			clipboard->setText(copy_data);
		break;
		case ASM_LONG_TABLE:
			for(int i = 0; i < nibble_count; i+=6){
				stream << "dl $" << hex_data[i] << hex_data[i+1] 
				       << hex_data[i+2] << hex_data[i+3]
				       << hex_data[i+4] << hex_data[i+5] << '\n';
			}
			copy_data.truncate(copy_data.length()-1);
			clipboard->setText(copy_data);
		break;
		case C_SOURCE:
			stream << "const unsigned char hex_data[] = {\n";
			for(int i = 0; i < nibble_count;){
				stream << '\t';
				for(int j = 0; j < 24 && i < nibble_count; j+=2, i+=2){
					stream << "0x" << hex_data[i] << hex_data[i+1] << ",";
				}
				copy_data.truncate(copy_data.length()-1);
				stream << ",\n";
			}
			copy_data.truncate(copy_data.length()-2);
			stream << "\n};";
			clipboard->setText(copy_data);
		break;
	}
}

void ROM_buffer::paste(int start, int end)
{
	if(check_paste_data()){
		return;
	}
	if(end){
		buffer.replace(start, end-start, QByteArray::fromHex(clipboard->text().toUtf8()));
		return;
	}
	buffer.insert(start, QByteArray::fromHex(clipboard->text().toUtf8()));
}

void ROM_buffer::delete_text(int start, int end)
{
	if(!end){
		buffer.remove(start, 1);
		return;
	}
	
	buffer.remove(start, end-start);
}

void ROM_buffer::update_nibble(char byte, int position)
{
	buffer[position/2] = (buffer.at(position/2) &
			     ((0x0F >> ((position & 1) << 2)) | (0x0F << ((position & 1) << 2)))) |
			     (byte << (((position & 1)^1) << 2));
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
