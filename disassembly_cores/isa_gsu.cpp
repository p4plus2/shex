#include "isa_gsu.h"
#include "utility.h"

isa_gsu::isa_gsu(QObject *parent) :
        disassembler_core(parent)
{
	set_alt->setValidator(new QIntValidator(0, 3, this));
			
	connect(set_alt, &QLineEdit::textEdited, this, &isa_gsu::change_alt);
	connect(stop, &QCheckBox::toggled, this, &isa_gsu::toggle_error_stop);
}

QGridLayout *isa_gsu::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(set_alt, 1, 0, 1, 1);
	grid->addWidget(stop, 1, 1, 2, 1);
	return grid;
}

template <typename V> 
QString isa_gsu::label_op(int offset, int size, V validator)
{
	QByteArray little_endian = QByteArray::fromHex(QByteArray(to_hex(offset, size).toLatin1()));
	int address = buffer->snes_to_pc((buffer->*validator)(delta * 2 + get_base(), little_endian));
	if(!in_range(address)){
		return '$' + to_hex(offset, size);
	}
	return add_label(address);
}

QString isa_gsu::decode_name_arg(const char arg, int &size)
{
	QString decode;
	decode.reserve(7);
	unsigned int operand = get_operand(0) | get_operand(1) | get_operand(2);
	switch(arg){
		case 'W':
			size += 2;
			return to_hex((operand & 0x00FFFF) << 1, 4);
		case 'w':
			size += 2;
			return to_hex(operand & 0x00FFFF, 4);
		case 'b':
			size++;
			return to_hex(operand & 0x0000FF, 2);	
		case 'r':
			size++;
			return label_op(operand & 0x0000FF, 2, &ROM_buffer::branch_address);
		case 'o':
			return QString::number(get_instruction() & 0x0F);
		case '1':
		case '2':
		case '3':
			alt_state = arg - '0';
			return "";
		default:
			qFatal("Invalid name decode arg");
	}
	return "";
}

QString isa_gsu::address_to_label(int address)
{
	return to_hex(buffer->pc_to_snes(address), 6);
}

QString isa_gsu::format_data_value(int size, int value, bool is_pointer)
{
	if(!is_pointer){
		return '$' + to_hex(value, (size+1)*2);
	}else{
		return label_op(value, (size+1)*2, &ROM_buffer::jump_address);
	}
}

disassembler_core::opcode isa_gsu::get_opcode(int op)
{
	if(alt_state && alt_states[alt_state]->contains(op)){
		int alt = alt_state;
		return alt_states[(alt_state = 0), alt]->value(op);
	}
	return opcode_list[op];
}

int isa_gsu::get_base()
{
	return region.get_start_byte();
}

bool isa_gsu::abort_unlikely(int op)
{
	return error_stop && unlikely.contains(op);
}

void isa_gsu::update_state()
{
	set_alt->setText(QString::number(alt_state));
}

isa_gsu::~isa_gsu()
{
	delete stop;
	delete set_alt;
}

#define OP1(O) {O}
#define OP2(O) OP1(O), OP1(O)
#define OP4(O) OP2(O), OP2(O)
#define OP6(O) OP4(O), OP2(O)
#define OP12(O) OP6(O), OP6(O)
#define OP15(O) OP12(O), OP2(O), OP1(O)
#define OP16(O) OP12(O), OP4(O)

const QList<disassembler_core::opcode> isa_gsu::opcode_list = {
	OP1 ("stop"),
	OP1 ("nop"),
	OP1 ("cache"),
	OP1 ("lsr"),
	OP1 ("rol"),
	OP1 ("bra %r"),
	OP1 ("blt %r"),
	OP1 ("bge %r"),
	OP1 ("bne %r"),
	OP1 ("beq %r"),
	OP1 ("bpl %r"),
	OP1 ("bmi %r"),
	OP1 ("bcc %r"),
	OP1 ("bcs %r"),
	OP1 ("bvc %r"),
	OP1 ("bvs %r"),
	OP16("to r%o"),
	OP16("with r%o"),
	OP12("stw (r%o)"),
	OP1 ("loop"),
	OP1 ("%1"),
	OP1 ("%2"),
	OP1 ("%3"),
	OP12("ldw (r%o)"),
	OP1 ("plot"),
	OP1 ("swap"),
	OP1 ("color"),
	OP1 ("not"),
	OP16("add r%o"),
	OP16("sub r%o"),
	OP1 ("merge"),
	OP15("and r%o"),
	OP16("mult r%o"),
	OP1 ("sbk"),
	OP4 ("link #%o"),
	OP1 ("sex"),
	OP1 ("asr"),
	OP1 ("ror"),
	OP6 ("jmp r%o"),
	OP1 ("lob"),
	OP1 ("fmult"),
	OP16("ibt r%o,#$%b"),
	OP16("from r%o"),
	OP1 ("hib"),
	OP15("or r%o"),
	OP15("inc r%o"),
	OP1 ("getc"),
	OP15("dec r%o"),
	OP1 ("getb"),
	OP16("iwt r%o,#$%w")
};

#undef OP1
#undef OP2
#undef OP4
#undef OP6
#undef OP12
#undef OP15
#undef OP16

#define OP1(I, O) {I, {O}}
#define OP2(I, O) OP1(I, O), OP1(I + 1, O)
#define OP4(I, O) OP2(I, O), OP2(I + 2, O)
#define OP6(I, O) OP4(I, O), OP2(I + 4, O)
#define OP12(I, O) OP6(I, O), OP6(I + 6, O)
#define OP15(I, O) OP12(I, O), OP2(I + 12, O), {I+14, {O}}
#define OP16(I, O) OP12(I, O), OP4(I + 12, O)

const QMap<unsigned char, disassembler_core::opcode> isa_gsu::alt1 = {
	OP12(0x30, "stb (r%o)"),
	OP12(0x40, "ldb (r%o)"),
	OP1 (0x4C, "rpix"),
	OP1 (0x4E, "cmode"),
	OP16(0x50, "adc r%o"),
	OP16(0x60, "sbc r%o"),
	OP15(0x71, "bic r%o"),
	OP16(0x80, "umult r%o"),
	OP1 (0x96, "div2"),
	OP6 (0x98, "ljmp r%o"),
	OP1 (0x9F, "lmult"),
	OP16(0xA0, "lms r%o,(#$%W)"),
	OP1 (0xC0, "hib"),
	OP15(0xC1, "xor r%o"),
	OP1 (0xEF, "getbh"),
	OP16(0xF0, "lm r%o")
};

const QMap<unsigned char, disassembler_core::opcode> isa_gsu::alt2 = {
	OP16(0x50, "add #%o"),
	OP16(0x60, "sub #%o"),
	OP15(0x71, "and #%o"),
	OP16(0x80, "mult #%o"),
	OP16(0xA0, "sms r%o,(#$%W)"),
	OP1 (0xC0, "hib"),
	OP15(0xC1, "or #%o"),
	OP1 (0xDF, "ramb"),
	OP1 (0xEF, "getbl"), 
	OP16(0xF0, "sm r%o")
};

const QMap<unsigned char, disassembler_core::opcode> isa_gsu::alt3 = {
	OP12(0x30, "stb (r%o)"),
	OP12(0x40, "ldb (r%o)"),
	OP1 (0x4C, "rpix"),
	OP1 (0x4E, "cmode"),
	OP16(0x50, "adc #%o"),
	OP16(0x60, "cmp r%o"),
	OP15(0x71, "bic #%o"),
	OP16(0x80, "umult #%o"),
	OP1 (0x96, "div2"),
	OP6 (0x98, "ljmp r%o"),
	OP1 (0x9f, "lmult"),
	OP16(0xA0, "lms r%o"),
	OP1 (0xC0, "hib"),
	OP15(0xC1, "xor #%o"),
	OP1 (0xDF, "romb"),
	OP1 (0xEF, "getbs"),
	OP16(0xF0, "lm r%o")
};

const QSet<unsigned char> isa_gsu::unlikely = {
	0x00
};

#undef OP1
#undef OP2
#undef OP4
#undef OP6
#undef OP12
#undef OP15
#undef OP16
