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
				add_data(opcode_address, make_table(data, delta, bookmark.size, width, packed));
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
		add_data(opcode_address, op.name);
		
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
	if(b.label.isEmpty()){
		label_id++;
		b.label = QString("label_") + address_to_label(destination);
	}
	return b.label;
}

void disassembler_core::add_data(int destination, QString data)
{
	disassembly_list[get_base()+destination].data = data;
}

QString disassembler_core::disassembly_text()
{
	QString text;
	foreach(block b, disassembly_list){
		if(!b.label.isEmpty()){
			text += b.label % ":\n";
		}
		if(!b.data.isEmpty()){
			text += (label_id > 0 ? "\t" : "") % b.data % "\n";
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

QString disassembler_core::make_table(QByteArray &data, int start, int size, int width, bool packed)
{
	const QString prefix[] = {"db $", "dw $", "dl $", "dd $"};
	const int packed_size[] = {8, 8, 9, 8};
	QString table;
	table.reserve(data.size() * 6);  // echo to hold ", $FF\n" for every byte
	add_label(start);
	add_label(start+size);
	for(int i = 0; i < size && i < start+data.size(); i += width+1){
		if(!packed || !(i % packed_size[width])){
			table.chop(3);
			table += "\n\t" % prefix[width];
		}
		
		unsigned int table_value = 0;
		for(int j = width; j >= 0; j--){
			table_value |= (unsigned char)data.at(start+i+j) << (j * 8);
		}
		table += QString::number(table_value, 16).rightJustified((width+1)*2, '0') % ", $";
	}
	table.chop(3);
	delta += size;
	return table.remove(0, 2);
}
