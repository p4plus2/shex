#include <QStringBuilder>

#include "disassembler_core.h"
#include "debug.h"

QString disassembler_core::disassemble(selection selection_area, const ROM_buffer *b)
{
	reset();
	QString error;
	buffer = b;
	region = selection_area;
	data = buffer->range(region.get_start_aligned(), region.get_end_aligned());
	delta = 0;
	
	const bookmark_map *bookmarks = buffer->get_bookmark_map();
	while(delta < data.size() && error.isEmpty()){
		int opcode_address = delta;
		const QString address = buffer->get_formatted_address(get_base()+delta);
		if(bookmarks->contains(address)){
			bookmark_data bookmark = bookmarks->value(address);
			if(bookmark.data_type & bookmark_data::CODE &&
			   !(bookmark.data_type & bookmark_data::UNKNOWN)){
				set_flags(bookmark.data_type);
			}else if(!(bookmark.data_type & bookmark_data::CODE)){
				bool packed = bookmark.data_type & bookmark_data::PACKED;
				int width = 0; //byte
				if(bookmark.data_type & bookmark_data::WORD){
					width = 1;
				}else if(bookmark.data_type & bookmark_data::LONG){
					width = 2;
				}else if(bookmark.data_type & bookmark_data::DOUBLE){
					width = 3;
				}
				make_table(data, delta, bookmark.size, width, packed);
				continue;
			}
		}
		unsigned char hex = data.at(delta);
		disassembler_core::opcode op = get_opcode(hex);	
		if(abort_unlikely(hex)){
			error = "Unlikely opcode detected, aborting!";
		}
		delta++;
		decode_name_args(op.name);
		add_data(opcode_address, op.name, block::CODE);
		
		if(delta > data.size()){
			error = "Disassembly range too small, last opcode may be invalid.";
		}
	}
	
	update_state();
	return disassembly_text() + error;
}

QString disassembler_core::add_label(int destination)
{
	block &b = disassembly_list[destination];
	qDebug() << destination;
	if(b.label.isEmpty()){
		label_id++;
		b.label = QString("label_") + address_to_label(destination);
	}
	return b.label;
}

QString disassembler_core::disassembly_text()
{
	const QString prefix[] = {"db ", "dw ", "dl ", "dd "};
	QString text;
	QString table_line;
	table_line.reserve(42);
	foreach(block b, disassembly_list){
		if(!b.label.isEmpty()){
			table_line.chop(2);
			text += table_line % '\n' % b.label % ":\n";
			table_line.clear();
		}
		if(b.data.isEmpty()){
			continue;
		}
		switch(b.format){
			case block::CODE:
				text += (label_id > 0 ? '\t' : '\0') % b.data % '\n';
			break;
			case block::DATA_PACKED ... block::DATA_PACKED_END:
				if(table_line.isEmpty()){
					table_line = prefix[b.format - block::DATA_PACKED];
				}
				table_line += '$' % b.data % ", ";
				if(table_line.length() > 42){
					table_line.chop(2);
					text += table_line % '\n';
					table_line.clear();
				}
			break;
			case block::DATA_UNPACKED ... block::DATA_UNPACKED_END:
			break;
				text += prefix[b.format - block::DATA_UNPACKED] % '$' % b.data % '\n';
			default:
				qDebug() << "Unknown formatter" << b.format;
			break;
		}
	}
	return text;
}

void disassembler_core::reset()
{
	disassembly_list.clear();
	label_id = 0;
}

void disassembler_core::decode_name_args(QString &name)
{
	int length = name.length();
	QString encoded = name;
	name.clear();
	for(int i = 0; i < length; i++){
		int size = 0;
		if(encoded[i] != '%'){
			name += encoded[i];
		}else{
			char arg = encoded.at(++i).toLatin1();
			name += decode_name_arg(arg, size);
			delta += size;
		}
	}
}

QString disassembler_core::get_hex(int n, int bytes)
{
	return QString::number(n, 16).rightJustified(bytes, '0').toUpper();
}

unsigned int disassembler_core::get_operand(int n)
{
	return delta+n < data.size() ? ((unsigned char)data.at(delta+n) << n*8) : 0;
}

void disassembler_core::add_data(int destination, QString data, block::data_format format)
{
	disassembly_list[get_base()+destination].data = data;
	disassembly_list[get_base()+destination].format = format;
}

void disassembler_core::make_table(QByteArray &data, int start, int size, int width, bool packed)
{
	add_label(get_base() + start);
	add_label(get_base() + start + size);
	for(int i = 0; i < size && i < start+data.size(); i += width+1){
		unsigned int table_value = 0;
		for(int j = width; j >= 0; j--){
			table_value |= (unsigned char)data.at(start+i+j) << (j * 8);
		}
		add_data(start+i, QString::number(table_value, 16).rightJustified((width+1)*2, '0').toUpper(),
				  (block::data_format)((packed ? block::DATA_PACKED : block::DATA_UNPACKED) + width));
	}
	delta += size;
}
