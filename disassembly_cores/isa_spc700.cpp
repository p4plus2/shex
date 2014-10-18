#include "isa_spc700.h"
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
	bool ok; //ignorable, input mask ensures valid data
	base = new_base.toInt(&ok, 16);
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
			return "#$" + get_hex(operand & 0x0000FF, 2);
		case 'd':
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

void isa_spc700::update_state()
{
}

const QList<disassembler_core::opcode> isa_spc700::opcode_list = {
        {"nop"},
	{"tcall 0"},
	{"set0 %d"},
	{"bbs0 %d, %r"},
	{"or a, %d"},
	{"or a, %w"},
	{"or a, (x)"},
	{"or a, (%d+x)"},
	{"or a, %b"},
	{"or %d, %d"},
	{"or1 c, %m:%d"},
	{"asl %d"},
	{"asl %w"},
	{"push p"},
	{"tset %w, a"},
	{"brk"},
	{"bpl %r"},
	{"tcall 1"},
	{"clr0 %d"},
	{"bbc0 %d, %r"},
	{"or a, %d+x"},
	{"or a, %w"},
	{"or a, %w"},
	{"or a, (%d)+y"},
	{"or %i"},
	{"or (x), (y)"},
	{"decw %d"},
	{"asl %d+x"},
	{"asl a"},
	{"dec x"},
	{"cmp x, %w"},
	{"jmp (%w+x)"},
	{"clrp"},
	{"tcall 2"},
	{"set1 %d"},
	{"bbs1 %d, %r"},
	{"and a, %d"},
	{"and a, %w"},
	{"and a, (x)"},
	{"and a, (%d+x)"},
	{"and a, %b"},
	{"and %d, %d"},
	{"or1 c, !%m:%d"},
	{"rol %d"},
	{"rol %w"},
	{"push a"},
	{"cbne %d, %r"},
	{"bra %r"},
	{"bmi %r"},
	{"tcall 3"},
	{"clr1 %d"},
	{"bbc1 %d, %r"},
	{"and a, %d+x"},
	{"and a, %w"},
	{"and a, %w"},
	{"and a, (%d)+y"},
	{"and %i"},
	{"and (x), (y)"},
	{"incw %d"},
	{"rol %d+x"},
	{"rol a"},
	{"inc x"},
	{"cmp x, %d"},
	{"call %j"},
	{"setp"},
	{"tcall 4"},
	{"set2 %d"},
	{"bbs2 %d, %r"},
	{"eor a, %d"},
	{"eor a, %w"},
	{"eor a, (x)"},
	{"eor a, (%d+x)"},
	{"eor a, %b"},
	{"eor %d, %d"},
	{"and1 c, %m:%d"},
	{"lsr %d"},
	{"lsr %w"},
	{"push x"},
	{"tclr %w, a"},
	{"pcall $ff%b"},
	{"bvc %r"},
	{"tcall 5"},
	{"clr2 %d"},
	{"bbc2 %d, %r"},
	{"eor a, %d+x"},
	{"eor a, %w"},
	{"eor a, %w"},
	{"eor a, (%d)+y"},
	{"eor %i"},
	{"eor (x), (y)"},
	{"cmpw ya, %d"},
	{"lsr %d+x"},
	{"lsr a"},
	{"mov x, a"},
	{"cmp y, %w"},
	{"jmp %w"},
	{"clrc"},
	{"tcall 6"},
	{"set3 %d"},
	{"bbs3 %d, %r"},
	{"cmp a, %d"},
	{"cmp a, %w"},
	{"cmp a, (x)"},
	{"cmp a, (%d+x)"},
	{"cmp a, %b"},
	{"cmp %d, %d"},
	{"and1 c, !%m:%d"},
	{"ror %d"},
	{"ror %w"},
	{"push y"},
	{"dbnz %d, %r"},
	{"ret"},
	{"bvs %r"},
	{"tcall 7"},
	{"clr3 %d"},
	{"bbc3 %d, %r"},
	{"cmp a, %d+x"},
	{"cmp a, %w"},
	{"cmp a, %w"},
	{"cmp a, (%d)+y"},
	{"cmp %i"},
	{"cmp (x), (y)"},
	{"addw ya, %d"},
	{"ror %d+x"},
	{"ror a"},
	{"mov a, x"},
	{"cmp y, %d"},
	{"reti"},
	{"setc"},
	{"tcall 8"},
	{"set4 %d"},
	{"bbs4 %d, %r"},
	{"adc a, %d"},
	{"adc a, %w"},
	{"adc a, (x)"},
	{"adc a, (%d+x)"},
	{"adc a, %b"},
	{"adc %d, %d"},
	{"eor1 c, %m:%d"},
	{"dec %d"},
	{"dec %w"},
	{"mov y, %b"},
	{"pop p"},
	{"mov %i"},
	{"bcc %r"},
	{"tcall 9"},
	{"clr4 %d"},
	{"bbc4 %d, %r"},
	{"adc a, %d+x"},
	{"adc a, %w"},
	{"adc a, %w"},
	{"adc a, (%d)+y"},
	{"adc %i"},
	{"adc (x), (y)"},
	{"subw ya, %d"},
	{"dec %d+x"},
	{"dec a"},
	{"mov x, sp"},
	{"div ya, x"},
	{"xcn a"},
	{"ei"},
	{"tcall 10"},
	{"set5 %d"},
	{"bbs5 %d, %r"},
	{"sbc a, %d"},
	{"sbc a, %w"},
	{"sbc a, (x)"},
	{"sbc a, (%d+x)"},
	{"sbc a, %b"},
	{"sbc %d, %d"},
	{"mov1 c, %m:%d"},
	{"inc %d"},
	{"inc %w"},
	{"cmp y, %b"},
	{"pop a"},
	{"mov (x)+, a"},
	{"bcs %r"},
	{"tcall 11"},
	{"clr5 %d"},
	{"bbc5 %d, %r"},
	{"sbc a, %d+x"},
	{"sbc a, %w"},
	{"sbc a, %w"},
	{"sbc a, (%d)+y"},
	{"sbc %i"},
	{"sbc (x), (y)"},
	{"movw ya, %d"},
	{"inc %d+x"},
	{"inc a"},
	{"mov sp, x"},
	{"das a"},
	{"mov a, (x)+"},
	{"di"},
	{"tcall 12"},
	{"set6 %d"},
	{"bbs6 %d, %r"},
	{"mov %d, a"},
	{"mov %w, a"},
	{"mov (x), a"},
	{"mov (%d+x), a"},
	{"cmp x, %b"},
	{"mov %w, x"},
	{"mov1 %m:%d, c"},
	{"mov %d, y"},
	{"mov %w, y"},
	{"mov x, %b"},
	{"pop x"},
	{"mul ya"},
	{"bne %r"},
	{"tcall 13"},
	{"clr6 %d"},
	{"bbc6 %d, %r"},
	{"mov %d+x, a"},
	{"mov %w+x, a"},
	{"mov %w+y, a"},
	{"mov (%d)+y, a"},
	{"mov %d, x"},
	{"mov %d+y, x"},
	{"movw %d, ya"},
	{"mov %d+x, y"},
	{"dec y"},
	{"mov a, y"},
	{"cbne %d, %r"},
	{"daa a"},
	{"clrv"},
	{"tcall 14"},
	{"set7 %d"},
	{"bbs7 %d, %r"},
	{"mov a, %d"},
	{"mov a, %w"},
	{"mov a, (x)"},
	{"mov a, (%d+x)"},
	{"mov a, %b"},
	{"mov x, %w"},
	{"not1 c, %m:%d"},
	{"mov y, %d"},
	{"mov y, %w"},
	{"notc"},
	{"pop y"},
	{"sleep"},
	{"beq %r"},
	{"tcall 15"},
	{"clr7 %d"},
	{"bbc7 %d, %r"},
	{"mov a, %d+x"},
	{"mov a, %w"},
	{"mov a, %w"},
	{"mov a, (%d)+y"},
	{"mov x, %d"},
	{"mov x, %d+y"},
	{"mov %d, %d"},
	{"mov y, %d+x"},
	{"inc y"},
	{"mov y, a"},
	{"dbnz y, %r"},
	{"stop"}
};

const QSet<unsigned char> isa_spc700::unlikely = {

};
