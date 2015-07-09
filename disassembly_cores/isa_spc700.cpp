#include "isa_spc700.h"
#include "utility.h"
#include "debug.h"

isa_spc700::isa_spc700(QObject *parent) :
        disassembler_core(parent)
{
	connect(stop, &QCheckBox::toggled, this, &isa_spc700::toggle_error_stop);
	connect(base_input, &QLineEdit::textChanged, this, &isa_spc700::update_base);
	base_input->setInputMask("HHHH");
}

void isa_spc700::update_base(QString new_base)
{
	base = new_base.toInt(nullptr, 16);
}

QGridLayout *isa_spc700::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(base_text, 0, 0, 1, 1);
	grid->addWidget(base_input, 0, 1, 1, 2);
	grid->addWidget(stop, 1, 0, 2, 1);
	return grid;
}

QString isa_spc700::decode_name_arg(const char arg, int &size)
{
	QString decode;
	decode.reserve(7);
	unsigned int operand = get_operand(0) | get_operand(1) | get_operand(2);
	int relative = (char)(operand & 0xFF) + 2 + delta;
	switch(arg){
		case 'b':
			size++;
			return "$" + get_hex(operand & 0x0000FF, 2);
		case 'w':
			size += 2;
			return "$" + get_hex(operand & 0x00FFFF, 4);
		// m and i are a pair always used together
		case 'm':
			return "$" + get_hex(operand & 0x001FFF, 4);
		case 'i':
			size += 2;
			return "$" + get_hex((operand & 0x00E000) >> 13, 2);
		case 'c':
			size += 2;
			return "$"  + get_hex((operand & 0x00FF00) >> 8, 2) +
			       "#$" + get_hex((operand & 0x0000FF) >> 0, 2);
		case 'r':
			size++;
			operand &= 0xFF;
			if(relative < 0 || relative > data.size()){
				return '$' + get_hex(operand & 0xFF, 2);
			}
			return add_label(relative + base);
		case 'j':
			size += 2;
			operand &= 0xFFFF;
			if(operand < base || operand > base + data.size()){
				return '$' + get_hex(operand, 4);
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

disassembler_core::opcode isa_spc700::get_opcode(int op)
{
	return opcode_list[op];
}

int isa_spc700::get_base()
{
	return base;
}

bool isa_spc700::abort_unlikely(int op)
{
	return error_stop && unlikely.contains(op);
}

isa_spc700::~isa_spc700()
{
	delete stop;
	delete base_text;
	delete base_input;
}

const QList<disassembler_core::opcode> isa_spc700::opcode_list = {
        {"nop"},
	{"tcall 0"},
	{"set0 %b"},
	{"bbs0 %b, %r"},
	{"or a, %b"},
	{"or a, %w"},
	{"or a, (x)"},
	{"or a, (%b+x)"},
	{"or a, #%b"},
	{"or %b, %b"},
	{"or1 c, %m:%b"},
	{"asl %b"},
	{"asl %w"},
	{"push p"},
	{"tset %w, a"},
	{"brk"},
	{"bpl %r"},
	{"tcall 1"},
	{"clr0 %b"},
	{"bbc0 %b, %r"},
	{"or a, %b+x"},
	{"or a, %w"},
	{"or a, %w"},
	{"or a, (%b)+y"},
	{"or %i"},
	{"or (x), (y)"},
	{"decw %b"},
	{"asl %b+x"},
	{"asl a"},
	{"dec x"},
	{"cmp x, %w"},
	{"jmp (%w+x)"},
	{"clrp"},
	{"tcall 2"},
	{"set1 %b"},
	{"bbs1 %b, %r"},
	{"and a, %b"},
	{"and a, %w"},
	{"and a, (x)"},
	{"and a, (%b+x)"},
	{"and a, #%b"},
	{"and %b, %b"},
	{"or1 c, !%m:%b"},
	{"rol %b"},
	{"rol %w"},
	{"push a"},
	{"cbne %b, %r"},
	{"bra %r"},
	{"bmi %r"},
	{"tcall 3"},
	{"clr1 %b"},
	{"bbc1 %b, %r"},
	{"and a, %b+x"},
	{"and a, %w"},
	{"and a, %w"},
	{"and a, (%b)+y"},
	{"and %i"},
	{"and (x), (y)"},
	{"incw %b"},
	{"rol %b+x"},
	{"rol a"},
	{"inc x"},
	{"cmp x, %b"},
	{"call %j"},
	{"setp"},
	{"tcall 4"},
	{"set2 %b"},
	{"bbs2 %b, %r"},
	{"eor a, %b"},
	{"eor a, %w"},
	{"eor a, (x)"},
	{"eor a, (%b+x)"},
	{"eor a, #%b"},
	{"eor %b, %b"},
	{"and1 c, %m:%b"},
	{"lsr %b"},
	{"lsr %w"},
	{"push x"},
	{"tclr %w, a"},
	{"pcall $ff#%b"},
	{"bvc %r"},
	{"tcall 5"},
	{"clr2 %b"},
	{"bbc2 %b, %r"},
	{"eor a, %b+x"},
	{"eor a, %w"},
	{"eor a, %w"},
	{"eor a, (%b)+y"},
	{"eor %i"},
	{"eor (x), (y)"},
	{"cmpw ya, %b"},
	{"lsr %b+x"},
	{"lsr a"},
	{"mov x, a"},
	{"cmp y, %w"},
	{"jmp %w"},
	{"clrc"},
	{"tcall 6"},
	{"set3 %b"},
	{"bbs3 %b, %r"},
	{"cmp a, %b"},
	{"cmp a, %w"},
	{"cmp a, (x)"},
	{"cmp a, (%b+x)"},
	{"cmp a, #%b"},
	{"cmp %b, %b"},
	{"and1 c, !%m:%b"},
	{"ror %b"},
	{"ror %w"},
	{"push y"},
	{"dbnz %b, %r"},
	{"ret"},
	{"bvs %r"},
	{"tcall 7"},
	{"clr3 %b"},
	{"bbc3 %b, %r"},
	{"cmp a, %b+x"},
	{"cmp a, %w"},
	{"cmp a, %w"},
	{"cmp a, (%b)+y"},
	{"cmp %i"},
	{"cmp (x), (y)"},
	{"addw ya, %b"},
	{"ror %b+x"},
	{"ror a"},
	{"mov a, x"},
	{"cmp y, %b"},
	{"reti"},
	{"setc"},
	{"tcall 8"},
	{"set4 %b"},
	{"bbs4 %b, %r"},
	{"adc a, %b"},
	{"adc a, %w"},
	{"adc a, (x)"},
	{"adc a, (%b+x)"},
	{"adc a, #%b"},
	{"adc %b, %b"},
	{"eor1 c, %m:%b"},
	{"dec %b"},
	{"dec %w"},
	{"mov y, #%b"},
	{"pop p"},
	{"mov %i"},
	{"bcc %r"},
	{"tcall 9"},
	{"clr4 %b"},
	{"bbc4 %b, %r"},
	{"adc a, %b+x"},
	{"adc a, %w"},
	{"adc a, %w"},
	{"adc a, (%b)+y"},
	{"adc %i"},
	{"adc (x), (y)"},
	{"subw ya, %b"},
	{"dec %b+x"},
	{"dec a"},
	{"mov x, sp"},
	{"div ya, x"},
	{"xcn a"},
	{"ei"},
	{"tcall 10"},
	{"set5 %b"},
	{"bbs5 %b, %r"},
	{"sbc a, %b"},
	{"sbc a, %w"},
	{"sbc a, (x)"},
	{"sbc a, (%b+x)"},
	{"sbc a, #%b"},
	{"sbc %b, %b"},
	{"mov1 c, %m:%b"},
	{"inc %b"},
	{"inc %w"},
	{"cmp y, #%b"},
	{"pop a"},
	{"mov (x)+, a"},
	{"bcs %r"},
	{"tcall 11"},
	{"clr5 %b"},
	{"bbc5 %b, %r"},
	{"sbc a, %b+x"},
	{"sbc a, %w"},
	{"sbc a, %w"},
	{"sbc a, (%b)+y"},
	{"sbc %i"},
	{"sbc (x), (y)"},
	{"movw ya, %b"},
	{"inc %b+x"},
	{"inc a"},
	{"mov sp, x"},
	{"das a"},
	{"mov a, (x)+"},
	{"di"},
	{"tcall 12"},
	{"set6 %b"},
	{"bbs6 %b, %r"},
	{"mov %b, a"},
	{"mov %w, a"},
	{"mov (x), a"},
	{"mov (%b+x), a"},
	{"cmp x, #%b"},
	{"mov %w, x"},
	{"mov1 %m:%b, c"},
	{"mov %b, y"},
	{"mov %w, y"},
	{"mov x, #%b"},
	{"pop x"},
	{"mul ya"},
	{"bne %r"},
	{"tcall 13"},
	{"clr6 %b"},
	{"bbc6 %b, %r"},
	{"mov %b+x, a"},
	{"mov %w+x, a"},
	{"mov %w+y, a"},
	{"mov (%b)+y, a"},
	{"mov %b, x"},
	{"mov %b+y, x"},
	{"movw %b, ya"},
	{"mov %b+x, y"},
	{"dec y"},
	{"mov a, y"},
	{"cbne %b, %r"},
	{"daa a"},
	{"clrv"},
	{"tcall 14"},
	{"set7 %b"},
	{"bbs7 %b, %r"},
	{"mov a, %b"},
	{"mov a, %w"},
	{"mov a, (x)"},
	{"mov a, (%b+x)"},
	{"mov a, #%b"},
	{"mov x, %w"},
	{"not1 c, %m:%b"},
	{"mov y, %b"},
	{"mov y, %w"},
	{"notc"},
	{"pop y"},
	{"sleep"},
	{"beq %r"},
	{"tcall 15"},
	{"clr7 %b"},
	{"bbc7 %b, %r"},
	{"mov a, %b+x"},
	{"mov a, %w"},
	{"mov a, %w"},
	{"mov a, (%b)+y"},
	{"mov x, %b"},
	{"mov x, %b+y"},
	{"mov %b, %b"},
	{"mov y, %b+x"},
	{"inc y"},
	{"mov y, a"},
	{"dbnz y, %r"},
	{"stop"}
};

const QSet<unsigned char> isa_spc700::unlikely = {

};
