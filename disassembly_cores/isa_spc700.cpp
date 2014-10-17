#include "isa_spc700.h"

isa_spc700::isa_spc700(QObject *parent) :
        disassembler_core(parent)
{
	connect(stop, &QCheckBox::toggled, this, &isa_spc700::toggle_error_stop);
}

QGridLayout *isa_spc700::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(stop, 1, 1, 2, 1);
	return grid;
}

#define label_op(O, A) {\
	if((O) < region.get_start_aligned() || (O) > region.get_end_aligned()){ \
		return '$' + get_hex(O, A); \
	}else{ \
		return add_label(O); \
	}}

QString isa_spc700::decode_name_arg(const char arg, int &size)
{
	QString decode;
	decode.reserve(7);
	unsigned int operand = get_operand(0) | get_operand(1) | get_operand(2);
	switch(arg){
		case 'b':
			size++;
			return "$" + get_hex(operand & 0x0000FF, 2);	
		default:
			qFatal("Invalid name decode arg");
	}
	return "";
}

#undef label_op

disassembler_core::opcode isa_spc700::get_opcode(int op)
{
	return opcode_list[op];
}

bool isa_spc700::abort_unlikely(int op)
{
	return error_stop && unlikely.contains(op);
}

void isa_spc700::update_state()
{
}

const QList<disassembler_core::opcode> isa_spc700::opcode_list = {

};

const QSet<unsigned char> isa_spc700::unlikely = {

};
