#include <QStringBuilder>

#include "utility.h"
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
	const QVector<int> rats_tags = buffer->get_rats_tags();

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
				make_table(data, delta, bookmark.size, width, packed, bookmark.data_is_pointer);
				continue;
			}
		}else if(rats_tags.contains(region.get_start_byte() + delta) && delta + 8 < data.size()){
			add_data(delta, "STAR", block::DATA_STRING);
			add_data(delta + 4, to_hex(read_word(data, delta + 4), 4), 
						   (block::data_format)(block::DATA_UNPACKED + 1));
			add_data(delta + 6, to_hex(read_word(data, delta + 6), 4), 
						   (block::data_format)(block::DATA_UNPACKED + 1));
			
			add_label(get_base() + delta, "RATS_tag_");
			delta += 8;
			add_label(get_base() + delta, "RATS_start_");
			add_label(get_base() + delta + read_word(data, delta - 4) + 1, "RATS_end_");
			continue;
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

QString disassembler_core::add_label(int destination, QString prefix)
{
	block &b = disassembly_list[destination];
	if(b.label.isEmpty()){
		label_id++;
		b.label = (prefix.isEmpty() ? "label_" : prefix) % address_to_label(destination) % ":\n";
	}
	QString label(b.label);
	label.chop(2);
	return label;
}

QString disassembler_core::disassembly_text()
{
	const QString prefix[] = {"\tdb ", "\tdw ", "\tdl ", "\tdd "};
	QString text;
	QString table_line;
	table_line.reserve(42);
	for(const auto &b : disassembly_list){
		if(!b.label.isEmpty() || table_line.length() > 42){
			table_line.chop(2);
			text += table_line % '\n' % b.label;
			table_line.clear();
		}
		if(b.data.isEmpty()){
			continue;
		}
		switch(b.format){
			case block::CODE:
				text += (label_id > 0 ? "\t" : "") % b.data % '\n';
			break;
			case block::DATA_PACKED ... block::DATA_PACKED_END:
				if(table_line.isEmpty()){
					table_line = prefix[b.format - block::DATA_PACKED];
				}
				table_line += b.data % ", ";
			break;
			case block::DATA_UNPACKED ... block::DATA_UNPACKED_END:
				text += prefix[b.format - block::DATA_UNPACKED] % b.data % '\n';
			break;
			case block::DATA_STRING:
				text += prefix[0] % '"' % b.data % "\"\n";
			break;
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

unsigned int disassembler_core::get_instruction()
{
	return (unsigned int)data.at(delta-1);
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

void disassembler_core::make_table(QByteArray &data, int start, int size, int width, bool packed, bool is_pointer)
{
	add_label(get_base() + start);
	add_label(get_base() + start + size);
	for(int i = 0; i < size && i + start < data.size(); i += width+1){
		unsigned int table_value = 0;
		for(int j = width; j >= 0 && (i + j + start) < data.size(); j--){
			table_value |= (unsigned char)data.at(start+i+j) << (j * 8);
		}
		add_data(start+i, format_data_value(width, table_value, is_pointer),
			  (block::data_format)((packed ? block::DATA_PACKED : block::DATA_UNPACKED) + width));
	}
	delta += size;
}
