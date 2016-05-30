#include "isa_spc700.h"
#include "utility.h"
#include "debug.h"

QString isa_spc700::decode_name_arg(const char arg, int &size)
{
	QString decode;
	decode.reserve(7);
	unsigned int operand = get_operand(0) | get_operand(1) | get_operand(2);
	int relative = (char)(operand & 0xFF) + 2 + delta;
	switch(arg){
		case 'b':
			size++;
			return "$" + to_hex(operand & 0x0000FF, 2);
		case 'w':
			size += 2;
			return "$" + to_hex(operand & 0x00FFFF, 4);
		// m and i are a pair always used together
		case 'm':
			return "$" + to_hex(operand & 0x001FFF, 4);
		case 'i':
			size += 2;
			return "$" + to_hex((operand & 0x00E000) >> 13, 2);
		case 'c':
			size += 2;
			return "$"  + to_hex((operand & 0x00FF00) >> 8, 2) +
			       "#$" + to_hex((operand & 0x0000FF) >> 0, 2);
		case 'r':
			size++;
			operand &= 0xFF;
			if(relative < 0 || relative > data.size()){
				return '$' + to_hex(operand & 0xFF, 2);
			}
			return add_label(relative + base);
		case 'j':
			size += 2;
			operand &= 0xFFFF;
			if(operand < base || operand > base + data.size()){
				return '$' + to_hex(operand, 4);
			}
			return add_label(operand);
		default:
			qFatal("Invalid name decode arg");
	}
	return "";
}

QString isa_spc700::address_to_label(int address)
{
	return to_hex(address, 4);
}

QString isa_spc700::format_data_value(int size, int value, bool is_pointer)
{
	if(!is_pointer){
		return '$' + to_hex(value, (size+1)*2);
	}else{
		if((unsigned int)value < base || (unsigned int)value > base + data.size()){
			return '$' + to_hex(value, 4);
		}
		return add_label(value);
	}
}

disassembler_core::opcode isa_spc700::get_opcode(int op)
{
	return opcode_list[op];
}

int isa_spc700::get_base()
{
	return base;
}

bool isa_spc700::is_unlikely_opcode(int op)
{
	return error_stop && unlikely.contains(op);
}

isa_spc700_ui::isa_spc700_ui(QObject *parent) :
        disassembler_core_ui(parent)
{
	set_disassembler(disassembler_list->get_factory("isa_spc700")->get_core(this));
	base_input->setInputMask("HHHH");
}

void isa_spc700_ui::update_base(QString new_base)
{
	((isa_spc700 *)disassembler())->base = new_base.toInt(nullptr, 16);
}

QGridLayout *isa_spc700_ui::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(base_text, 0, 0, 1, 1);
	grid->addWidget(base_input, 0, 1, 1, 2);
	grid->addWidget(stop, 1, 0, 2, 1);
	return grid;
}

isa_spc700_ui::~isa_spc700_ui()
{
	delete stop;
	delete base_text;
	delete base_input;
}

using hint = disassembler_core::opcode;

const QList<disassembler_core::opcode> isa_spc700::opcode_list = {
        {"nop",			hint::NONE},
	{"tcall 0",		hint::NONE},
	{"set0 %b",		hint::NONE},
	{"bbs0 %b, %r",		hint::NONE},
	{"or a, %b",		hint::NONE},
	{"or a, %w",		hint::NONE},
	{"or a, (x)",		hint::NONE},
	{"or a, (%b+x)",	hint::NONE},
	{"or a, #%b",		hint::NONE},
	{"or %b, %b",		hint::NONE},
	{"or1 c, %m:%b",	hint::NONE},
	{"asl %b",		hint::NONE},
	{"asl %w",		hint::NONE},
	{"push p",		hint::NONE},
	{"tset %w, a",		hint::NONE},
	{"brk",			hint::NONE},
	{"bpl %r",		hint::NONE},
	{"tcall 1",		hint::NONE},
	{"clr0 %b",		hint::NONE},
	{"bbc0 %b, %r",		hint::NONE},
	{"or a, %b+x",		hint::NONE},
	{"or a, %w",		hint::NONE},
	{"or a, %w",		hint::NONE},
	{"or a, (%b)+y",	hint::NONE},
	{"or %i",		hint::NONE},
	{"or (x), (y)",		hint::NONE},
	{"decw %b",		hint::NONE},
	{"asl %b+x",		hint::NONE},
	{"asl a",		hint::NONE},
	{"dec x",		hint::NONE},
	{"cmp x, %w",		hint::NONE},
	{"jmp (%w+x)",		hint::NONE},
	{"clrp",		hint::NONE},
	{"tcall 2",		hint::NONE},
	{"set1 %b",		hint::NONE},
	{"bbs1 %b, %r",		hint::NONE},
	{"and a, %b",		hint::NONE},
	{"and a, %w",		hint::NONE},
	{"and a, (x)",		hint::NONE},
	{"and a, (%b+x)",	hint::NONE},
	{"and a, #%b",		hint::NONE},
	{"and %b, %b",		hint::NONE},
	{"or1 c, !%m:%b",	hint::NONE},
	{"rol %b",		hint::NONE},
	{"rol %w",		hint::NONE},
	{"push a",		hint::NONE},
	{"cbne %b, %r",		hint::NONE},
	{"bra %r",		hint::NONE},
	{"bmi %r",		hint::NONE},
	{"tcall 3",		hint::NONE},
	{"clr1 %b",		hint::NONE},
	{"bbc1 %b, %r",		hint::NONE},
	{"and a, %b+x",		hint::NONE},
	{"and a, %w",		hint::NONE},
	{"and a, %w",		hint::NONE},
	{"and a, (%b)+y",	hint::NONE},
	{"and %i",		hint::NONE},
	{"and (x), (y)",	hint::NONE},
	{"incw %b",		hint::NONE},
	{"rol %b+x",		hint::NONE},
	{"rol a",		hint::NONE},
	{"inc x",		hint::NONE},
	{"cmp x, %b",		hint::NONE},
	{"call %j",		hint::NONE},
	{"setp",		hint::NONE},
	{"tcall 4",		hint::NONE},
	{"set2 %b",		hint::NONE},
	{"bbs2 %b, %r",		hint::NONE},
	{"eor a, %b",		hint::NONE},
	{"eor a, %w",		hint::NONE},
	{"eor a, (x)",		hint::NONE},
	{"eor a, (%b+x)",	hint::NONE},
	{"eor a, #%b",		hint::NONE},
	{"eor %b, %b",		hint::NONE},
	{"and1 c, %m:%b",	hint::NONE},
	{"lsr %b",		hint::NONE},
	{"lsr %w",		hint::NONE},
	{"push x",		hint::NONE},
	{"tclr %w, a",		hint::NONE},
	{"pcall $ff#%b",	hint::NONE},
	{"bvc %r",		hint::NONE},
	{"tcall 5",		hint::NONE},
	{"clr2 %b",		hint::NONE},
	{"bbc2 %b, %r",		hint::NONE},
	{"eor a, %b+x",		hint::NONE},
	{"eor a, %w",		hint::NONE},
	{"eor a, %w",		hint::NONE},
	{"eor a, (%b)+y",	hint::NONE},
	{"eor %i",		hint::NONE},
	{"eor (x), (y)",	hint::NONE},
	{"cmpw ya, %b",		hint::NONE},
	{"lsr %b+x",		hint::NONE},
	{"lsr a",		hint::NONE},
	{"mov x, a",		hint::NONE},
	{"cmp y, %w",		hint::NONE},
	{"jmp %w",		hint::NONE},
	{"clrc",		hint::NONE},
	{"tcall 6",		hint::NONE},
	{"set3 %b",		hint::NONE},
	{"bbs3 %b, %r",		hint::NONE},
	{"cmp a, %b",		hint::NONE},
	{"cmp a, %w",		hint::NONE},
	{"cmp a, (x)",		hint::NONE},
	{"cmp a, (%b+x)",	hint::NONE},
	{"cmp a, #%b",		hint::NONE},
	{"cmp %b, %b",		hint::NONE},
	{"and1 c, !%m:%b",	hint::NONE},
	{"ror %b",		hint::NONE},
	{"ror %w",		hint::NONE},
	{"push y",		hint::NONE},
	{"dbnz %b, %r",		hint::NONE},
	{"ret",			hint::NONE},
	{"bvs %r",		hint::NONE},
	{"tcall 7",		hint::NONE},
	{"clr3 %b",		hint::NONE},
	{"bbc3 %b, %r",		hint::NONE},
	{"cmp a, %b+x",		hint::NONE},
	{"cmp a, %w",		hint::NONE},
	{"cmp a, %w",		hint::NONE},
	{"cmp a, (%b)+y",	hint::NONE},
	{"cmp %i",		hint::NONE},
	{"cmp (x), (y)",	hint::NONE},
	{"addw ya, %b",		hint::NONE},
	{"ror %b+x",		hint::NONE},
	{"ror a",		hint::NONE},
	{"mov a, x",		hint::NONE},
	{"cmp y, %b",		hint::NONE},
	{"reti",		hint::NONE},
	{"setc",		hint::NONE},
	{"tcall 8",		hint::NONE},
	{"set4 %b",		hint::NONE},
	{"bbs4 %b, %r",		hint::NONE},
	{"adc a, %b",		hint::NONE},
	{"adc a, %w",		hint::NONE},
	{"adc a, (x)",		hint::NONE},
	{"adc a, (%b+x)",	hint::NONE},
	{"adc a, #%b",		hint::NONE},
	{"adc %b, %b",		hint::NONE},
	{"eor1 c, %m:%b",	hint::NONE},
	{"dec %b",		hint::NONE},
	{"dec %w",		hint::NONE},
	{"mov y, #%b",		hint::NONE},
	{"pop p",		hint::NONE},
	{"mov %i",		hint::NONE},
	{"bcc %r",		hint::NONE},
	{"tcall 9",		hint::NONE},
	{"clr4 %b",		hint::NONE},
	{"bbc4 %b, %r",		hint::NONE},
	{"adc a, %b+x",		hint::NONE},
	{"adc a, %w",		hint::NONE},
	{"adc a, %w",		hint::NONE},
	{"adc a, (%b)+y",	hint::NONE},
	{"adc %i",		hint::NONE},
	{"adc (x), (y)",	hint::NONE},
	{"subw ya, %b",		hint::NONE},
	{"dec %b+x",		hint::NONE},
	{"dec a",		hint::NONE},
	{"mov x, sp",		hint::NONE},
	{"div ya, x",		hint::NONE},
	{"xcn a",		hint::NONE},
	{"ei",			hint::NONE},
	{"tcall 10",		hint::NONE},
	{"set5 %b",		hint::NONE},
	{"bbs5 %b, %r",		hint::NONE},
	{"sbc a, %b",		hint::NONE},
	{"sbc a, %w",		hint::NONE},
	{"sbc a, (x)",		hint::NONE},
	{"sbc a, (%b+x)",	hint::NONE},
	{"sbc a, #%b",		hint::NONE},
	{"sbc %b, %b",		hint::NONE},
	{"mov1 c, %m:%b",	hint::NONE},
	{"inc %b",		hint::NONE},
	{"inc %w",		hint::NONE},
	{"cmp y, #%b",		hint::NONE},
	{"pop a",		hint::NONE},
	{"mov (x)+, a",		hint::NONE},
	{"bcs %r",		hint::NONE},
	{"tcall 11",		hint::NONE},
	{"clr5 %b",		hint::NONE},
	{"bbc5 %b, %r",		hint::NONE},
	{"sbc a, %b+x",		hint::NONE},
	{"sbc a, %w",		hint::NONE},
	{"sbc a, %w",		hint::NONE},
	{"sbc a, (%b)+y",	hint::NONE},
	{"sbc %i",		hint::NONE},
	{"sbc (x), (y)",	hint::NONE},
	{"movw ya, %b",		hint::NONE},
	{"inc %b+x",		hint::NONE},
	{"inc a",		hint::NONE},
	{"mov sp, x",		hint::NONE},
	{"das a",		hint::NONE},
	{"mov a, (x)+",		hint::NONE},
	{"di",			hint::NONE},
	{"tcall 12",		hint::NONE},
	{"set6 %b",		hint::NONE},
	{"bbs6 %b, %r",		hint::NONE},
	{"mov %b, a",		hint::NONE},
	{"mov %w, a",		hint::NONE},
	{"mov (x), a",		hint::NONE},
	{"mov (%b+x), a",	hint::NONE},
	{"cmp x, #%b",		hint::NONE},
	{"mov %w, x",		hint::NONE},
	{"mov1 %m:%b, c",	hint::NONE},
	{"mov %b, y",		hint::NONE},
	{"mov %w, y",		hint::NONE},
	{"mov x, #%b",		hint::NONE},
	{"pop x",		hint::NONE},
	{"mul ya",		hint::NONE},
	{"bne %r",		hint::NONE},
	{"tcall 13",		hint::NONE},
	{"clr6 %b",		hint::NONE},
	{"bbc6 %b, %r",		hint::NONE},
	{"mov %b+x, a",		hint::NONE},
	{"mov %w+x, a",		hint::NONE},
	{"mov %w+y, a",		hint::NONE},
	{"mov (%b)+y, a",	hint::NONE},
	{"mov %b, x",		hint::NONE},
	{"mov %b+y, x",		hint::NONE},
	{"movw %b, ya",		hint::NONE},
	{"mov %b+x, y",		hint::NONE},
	{"dec y",		hint::NONE},
	{"mov a, y",		hint::NONE},
	{"cbne %b, %r",		hint::NONE},
	{"daa a",		hint::NONE},
	{"clrv",		hint::NONE},
	{"tcall 14",		hint::NONE},
	{"set7 %b",		hint::NONE},
	{"bbs7 %b, %r",		hint::NONE},
	{"mov a, %b",		hint::NONE},
	{"mov a, %w",		hint::NONE},
	{"mov a, (x)",		hint::NONE},
	{"mov a, (%b+x)",	hint::NONE},
	{"mov a, #%b",		hint::NONE},
	{"mov x, %w",		hint::NONE},
	{"not1 c, %m:%b",	hint::NONE},
	{"mov y, %b",		hint::NONE},
	{"mov y, %w",		hint::NONE},
	{"notc",		hint::NONE},
	{"pop y",		hint::NONE},
	{"sleep",		hint::NONE},
	{"beq %r",		hint::NONE},
	{"tcall 15",		hint::NONE},
	{"clr7 %b",		hint::NONE},
	{"bbc7 %b, %r",		hint::NONE},
	{"mov a, %b+x",		hint::NONE},
	{"mov a, %w",		hint::NONE},
	{"mov a, %w",		hint::NONE},
	{"mov a, (%b)+y",	hint::NONE},
	{"mov x, %b",		hint::NONE},
	{"mov x, %b+y",		hint::NONE},
	{"mov %b, %b",		hint::NONE},
	{"mov y, %b+x",		hint::NONE},
	{"inc y",		hint::NONE},
	{"mov y, a",		hint::NONE},
	{"dbnz y, %r",		hint::NONE},
	{"stop",		hint::NONE}
};

const QSet<unsigned char> isa_spc700::unlikely = {

};
