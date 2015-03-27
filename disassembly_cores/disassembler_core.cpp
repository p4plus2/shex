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
	while(delta < data.size() && error.isEmpty()){
		int size = 0;
		unsigned char hex = data.at(delta);
		disassembler_core::opcode op = get_opcode(hex);	
		if(abort_unlikely(hex)){
			error = "Unlikely opcode detected, aborting!";
		}
		
		delta++;
		decode_name_args(op.name);
		add_mnemonic(get_base()+delta, op.name);
		
		delta += size;
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

void disassembler_core::add_mnemonic(int destination, QString mnemonic)
{
	disassembly_list[destination].mnemonic = mnemonic;
}

QString disassembler_core::disassembly_text()
{
	QString text;
	foreach(block b, disassembly_list){
		if(!b.label.isEmpty()){
			text += b.label % ":\n";
		}
		if(!b.mnemonic.isEmpty()){
			text += (label_id > 0 ? "\t" : "") % b.mnemonic % "\n";
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
