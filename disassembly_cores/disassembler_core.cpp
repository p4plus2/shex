#include <QStringBuilder>

#include "utility.h"
#include "disassembler_core.h"

#include "isa_65c816.h"
#include "isa_spc700.h"
#include "isa_gsu.h"

#include "debug.h"

QString disassembler_core::disassemble(selection selection_area, const ROM_buffer *b)
{
	reset();
	buffer = b;
	region = selection_area;
	data = buffer->range(region.get_start_aligned(), region.get_end_aligned());
	
	const bookmark_map *bookmarks = buffer->get_bookmark_map();
	const QVector<int> rats_tags = buffer->get_rats_tags();

	while(delta < data.size() && error.isEmpty()){
		const QString address = buffer->get_formatted_address(get_base()+delta);
		if(bookmarks->contains(address)){
			bookmark_data bookmark = bookmarks->value(address);
			if(bookmark.data_type & bookmark_data::CODE && !(bookmark.data_type & bookmark_data::UNKNOWN)){
				set_flags(bookmark.data_type);
			}else if(!(bookmark.data_type & bookmark_data::CODE)){
				disassemble_table(bookmark);
				continue;
			}
		}else if(rats_tags.contains(region.get_start_byte() + delta) && delta + 8 < data.size()){
			disassemble_rats();
			continue;
		}/*else if(delta == 3){
			qDebug() << 123;
			selection s = selection::create_selection(selection_area.get_start_aligned() + delta, 1);
			isa_spc700 test;
			qDebug() << test.disassemble(s, b);
			for(auto i = test.disassembly_list.begin(); i != test.disassembly_list.end(); i++){
				disassembly_list.insert(i.key(), i.value());
			}
			delta += 1;
			continue;
		}*/
		disassemble_code();
		//qDebug() << delta;
		
	}
	
	if(parent){
		update_state();
	}
	//qDebug() << disassembly_text() + error;
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
	error.clear();
	label_id = 0;
	delta = 0;
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

bool disassembler_core::in_range(int address)
{
	return region.get_start_byte() < address && region.get_end_byte() > address;
}

void disassembler_core::add_data(int destination, QString data, block::data_format format)
{
	disassembly_list[get_base()+destination].data = data;
	disassembly_list[get_base()+destination].format = format;
}

void disassembler_core::disassemble_table(const bookmark_data &bookmark)
{
	bool packed = bookmark.data_type & bookmark_data::PACKED;
	int width = (bookmark.data_type & bookmark_data::BYTE) ? 0 :
		    (bookmark.data_type & bookmark_data::WORD) ? 1 :
	            (bookmark.data_type & bookmark_data::LONG) ? 2 :
	                                                         3 ; //double
	add_label(get_base() + delta);
	add_label(get_base() + delta + bookmark.size, "\nlabel_");
	for(int i = 0; i < bookmark.size && i + delta < data.size(); i += width + 1){
		unsigned int table_value = 0;
		for(int j = width; j >= 0 && (i + j + delta) < data.size(); j--){
			table_value |= (unsigned char)data.at(delta+i+j) << (j * 8);
		}
		add_data(delta + i, format_data_value(width, table_value, bookmark.data_is_pointer),
			  (block::data_format)((packed ? block::DATA_PACKED : block::DATA_UNPACKED) + width));
	}
	delta += bookmark.size;
}

void disassembler_core::disassemble_rats()
{
	add_data(delta, "STAR", block::DATA_STRING);
	add_data(delta + 4, to_hex(read_word(data, delta + 4), 4), block::DATA_UNPACKED_WORD);
	add_data(delta + 6, to_hex(read_word(data, delta + 6), 4), block::DATA_UNPACKED_WORD);
	
	add_label(get_base() + delta, "RATS_tag_");
	delta += 8;
	add_label(get_base() + delta, "RATS_start_");
	add_label(get_base() + delta + read_word(data, delta - 4) + 1, "RATS_end_");
}

void disassembler_core::disassemble_code()
{
	int opcode_address = delta;
	unsigned char hex = data.at(delta);
	disassembler_core::opcode op = get_opcode(hex);	
	bool data_found = false;
	if(is_unlikely_opcode(hex) || 
		(previous_codeflow && is_semiunlikely_opcode(hex)) ||
		is_unlikely_operand()){
		data_found = disassemble_data();
	}
	
	if(!data_found){
		is_stateful_opcode(hex);
		delta++;
		decode_name_args(op.name);
		add_data(opcode_address, op.name, block::CODE);
		previous_codeflow = is_codeflow_opcode(hex);
	}
	
	if(delta > data.size()){
		error = "Disassembly range too small, last opcode may be invalid.";
	}
}

bool disassembler_core::disassemble_data()
{
	int start_address = delta;
	unsigned char hex = data.at(delta);
	disassembler_core::opcode op = get_opcode(hex);	
	
	bookmark_data bookmark;
	bookmark.data_type = (bookmark_data::types)(bookmark_data::PACKED | bookmark_data::BYTE);
	bookmark.data_is_pointer = false;
	
	const int valid_byte_count = 10;
	int is_valid = valid_byte_count;
	int potential_delta = delta;
	
	const int opcode_repeat_threshold = 5;
	int opcode_repeat = 0;
	int opcode_repeat_count = 0;
	
	auto check_pointers = [&](){
		const int max_distance[] = {0, 3, 2, 1};
		int max_depth = delta;
		for(int width = 3; width > 1; width--){
			int pointer_delta = delta;
			int base_byte = data.at(delta);
			int byte_count = 0;
			while(pointer_delta < data.size()){
				int current_byte = data.at(pointer_delta);
				if(base_byte - max_distance[width] < current_byte && 
				   current_byte < base_byte + max_distance[width]){
					byte_count++;
				}else{
					break;
				}
				pointer_delta += width;
			}
			
			if(pointer_delta > max_depth){
				max_depth = pointer_delta;
			}
		}
		max_depth -= delta;
		if(max_depth > 5){
			delta += max_depth;
			is_valid = valid_byte_count;
			return true;
		}
		return false;
	};
	
	while(delta < data.size()){
		hex = data.at(delta);
		
		if(is_valid == valid_byte_count){
			potential_delta = delta;
		}
		if(check_pointers()){
			continue;
		}
		
		
		//if(opcode_repeat == hex){
		//	opcode_repeat_count++;
		//	if(opcode_repeat_count > opcode_repeat_threshold){
		//		potential_delta = delta;
		//		is_valid = valid_byte_count;
		//	}
		//}else{
		//	opcode_repeat_count = 0;
		//}
		//opcode_repeat = hex;

		if(!is_unlikely_opcode(hex)){
			if(is_valid < 0 && !opcode_repeat_count &&
			   !is_semiunlikely_opcode(hex) && !is_unlikely_operand()){
				delta = potential_delta;
				break;
			}
			
			if(!is_semiunlikely_opcode(hex) && !is_unlikely_operand()){
				//decode opcode is likely
				push_state();
				delta++;
				op = get_opcode(hex);
				decode_name_args(op.name);
				delta--;
				pop_state();
			}
			is_valid--;
		}else{
			if(is_valid < valid_byte_count){
				is_valid = valid_byte_count;
				opcode_repeat_count = 0;
			}
		}
		delta++;
	}
	
	bookmark.size = delta - start_address;
	delta = start_address;
	if(bookmark.size){
		disassemble_table(bookmark);
		return true;
	}
	return false;
}

QString disassembler_core_ui::disassemble(selection selection_area, const ROM_buffer *b)
{
	return core->disassemble(selection_area, b);
}
