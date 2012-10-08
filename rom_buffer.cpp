#include "rom_buffer.h"

#include <QTextStream>

ROM_buffer::ROM_buffer()
{
	ROM.setFileName("smw.smc");
	ROM.open(QFile::ReadWrite);
	buffer = ROM.readAll();
	clipboard = QApplication::clipboard();
}

void ROM_buffer::cut(int start, int end)
{
	clipboard->setText(buffer.mid(start, end-start).toHex());
	buffer.remove(start, end-start);
}

void ROM_buffer::copy(int start, int end)
{	
	clipboard->setText(buffer.mid(start, end-start).toHex());
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
