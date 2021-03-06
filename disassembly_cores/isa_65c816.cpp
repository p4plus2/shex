#include "isa_65c816.h"
#include "utility.h"
#include "debug.h"

isa_65c816::isa_65c816(QObject *parent) :
        disassembler_core(parent)
{
	connect(this, &isa_65c816::A_changed, set_A, &QCheckBox::setChecked);
	connect(this, &isa_65c816::I_changed, set_I, &QCheckBox::setChecked);
			
	connect(set_A, &QCheckBox::toggled, this, &isa_65c816::toggle_A);
	connect(set_I, &QCheckBox::toggled, this, &isa_65c816::toggle_I);
	connect(stop, &QCheckBox::toggled, this, &isa_65c816::toggle_error_stop);
}

QGridLayout *isa_65c816::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(set_A, 1, 0, 1, 1);
	grid->addWidget(set_I, 2, 0, 1, 1);
	grid->addWidget(stop, 1, 1, 2, 1);
	return grid;
}

template <typename V> 
QString isa_65c816::label_op(int offset, int size, V validator)
{
	QByteArray little_endian = QByteArray::fromHex(QByteArray(to_hex(offset, size).toLatin1()));
	int address = buffer->snes_to_pc((buffer->*validator)(delta * 2 + get_base(), little_endian));
	if(!in_range(address)){
		return '$' + to_hex(offset, size);
	}
	return add_label(address);
}

QString isa_65c816::decode_name_arg(const char arg, int &size)
{
	QString decode;
	decode.reserve(7);
	unsigned int operand = get_operand(0) | get_operand(1) | get_operand(2);
	switch(arg){
		case 'l':
			size += 3;
			return "$" + to_hex(operand & 0xFFFFFF, 6);
		case 'w':
			size += 2;
			return "$" + to_hex(operand & 0x00FFFF, 4);
		case 'b':
			size++;
			return "$" + to_hex(operand & 0x0000FF, 2);	
		case 'r':
			size++;
			return label_op(operand & 0x0000FF, 2, &ROM_buffer::branch_address);
		case 'R':
			size += 2;
			return label_op(operand & 0x00FFFF, 4, &ROM_buffer::branch_address);
		case 'j':
			size += 2;
			return label_op(operand & 0x00FFFF, 4, &ROM_buffer::jump_address);
		case 'J':
			size += 3;
			return label_op(operand & 0xFFFFFF, 6, &ROM_buffer::jump_address);
		case 'a':
			size += 1 + A_state;
			if(A_state){
				return "#$" + to_hex(operand & 0x00FFFF, 4);	
			}else{
				return "#$" + to_hex(operand & 0x0000FF, 2);	
			}
		case 'i':
			size += 1 + I_state;
			if(I_state){
				return "#$" + to_hex(operand & 0x00FFFF, 4);	
			}else{
				return "#$" + to_hex(operand & 0x0000FF, 2);	
			}
		case 'f':
			A_state = (operand & 0x20) ? data.at(delta-1) == (char)0xC2 : A_state;
			I_state = (operand & 0x10) ? data.at(delta-1) == (char)0xC2 : I_state;
		case 'c':
			size++;
			return "#$" + to_hex(operand & 0x0000FF, 2);	
		default:
			qFatal("Invalid name decode arg");
	}
	return "";
}

QString isa_65c816::address_to_label(int address)
{
	return to_hex(buffer->pc_to_snes(address), 6);
}

QString isa_65c816::format_data_value(int size, int value, bool is_pointer)
{
	if(!is_pointer){
		return '$' + to_hex(value, (size+1)*2);
	}else{
		return label_op(value, (size+1)*2, &ROM_buffer::jump_address);
	}
}

disassembler_core::opcode isa_65c816::get_opcode(int op)
{
	return opcode_list[op];
}

int isa_65c816::get_base()
{
	return region.get_start_byte();
}

bool isa_65c816::abort_unlikely(int op)
{
	return error_stop && unlikely.contains(op);
}

void isa_65c816::update_state()
{
  	emit A_changed(A_state);
	emit I_changed(I_state);
}

void isa_65c816::set_flags(bookmark_data::types type)
{
	A_state = type & bookmark_data::A;
	I_state = type & bookmark_data::I;
}

isa_65c816::~isa_65c816()
{
	delete stop;
	delete set_A;
	delete set_I;
}

const QList<disassembler_core::opcode> isa_65c816::opcode_list = {
        {"BRK %c"},
	{"ORA (%b,X)"},
	{"COP %c"},
	{"ORA %b,s"},
	{"TSB %b"},
	{"ORA %b"},
	{"ASL %b"},
	{"ORA [%b]"},
	{"PHP"},
	{"ORA %a"},
	{"ASL A"},
	{"PHD"},
	{"TSB %w"},
	{"ORA %w"},
	{"ASL %w"},
	{"ORA %l"},
	{"BPL %r"},
	{"ORA (%b),Y"},
	{"ORA (%b)"},
	{"ORA (%b,S),Y"},
	{"TRB %b"},
	{"ORA %b,X"},
	{"ASL %b,X"},
	{"ORA [%b],Y"},
	{"CLC"},
	{"ORA %w,Y"},
	{"INC A"},
	{"TCS"},
	{"TRB %w"},
	{"ORA %w,X"},
	{"ASL %w,X"},
	{"ORA %l,X"},
	{"JSR %j"},
	{"AND (%b,X)"},
	{"JSL %J"},
	{"AND %b"},
	{"BIT %b"},
	{"AND %b"},
	{"ROL %b"},
	{"AND [%b]"},
	{"PLP"},
	{"AND %a"},
	{"ROL A"},
	{"PLD"},
	{"BIT %w"},
	{"AND %w"},
	{"ROL %w"},
	{"AND %l"},
	{"BMI %r"},
	{"AND (%b),Y"},
	{"AND (%b)"},
	{"AND (%b,S),Y"},
	{"BIT %b,X"},
	{"AND %b,X"},
	{"ROL %b,X"},
	{"AND [%b],Y"},
	{"SEC"},
	{"AND %w,Y"},
	{"DEC A"},
	{"TSC"},
	{"BIT %b,X"},
	{"AND %b,X"},
	{"ROL %b,X"},
	{"AND %l,X"},
	{"RTI"},
	{"EOR (%b,X)"},
	{"WDM %b"},
	{"EOR %b,S"},
	{"MVN %w"},
	{"EOR %b"},
	{"LSR %b"},
	{"EOR [%b]"},
	{"PHA"},
	{"EOR %a"},
	{"LSR A"},
	{"PHK"},
	{"JMP %j"},
	{"EOR %w"},
	{"LSR %w"},
	{"EOR %l"},
	{"BVC %r"},
	{"EOR (%b),Y"},
	{"EOR (%b)"},
	{"EOR (%b,S),Y"},
	{"MVN %w"},
	{"EOR %b,X"},
	{"LSR %b,X"},
	{"EOR [%b],Y"},
	{"CLI"},
	{"EOR %w,Y"},
	{"PHY"},
	{"TCD"},
	{"JML %J"},
	{"EOR %w,X"},
	{"LSR %w,X"},
	{"EOR %l,X"},
	{"RTS"},
	{"ADC (%b,X)"},
	{"PER %w"},
	{"ADC %b,S"},
	{"STZ %b"},
	{"ADC %b"},
	{"ROR %b"},
	{"ADC [%b]"},
	{"PLA"},
	{"ADC %a"},
	{"ROR A"},
	{"RTL"},
	{"JMP (%j)"},
	{"ADC %w"},
	{"ROR %w"},
	{"ADC %l"},
	{"BVS %r"},
	{"ADC (%b),Y"},
	{"ADC (%b)"},
	{"ADC (%b,S),Y"},
	{"STZ %b,X"},
	{"ADC %b,X"},
	{"ROR %b,X"},
	{"ADC [%b],Y"},
	{"SEI"},
	{"ADC %w,Y"},
	{"PLY"},
	{"TDC"},
	{"JMP (%j,X)"},
	{"ADC %w,X"},
	{"ROR %w,X"},
	{"ADC %l,X"},
	{"BRA %r"},
	{"STA (%b,X)"},
	{"BRL %R"},
	{"STA %b,S"},
	{"STY %b"},
	{"STA %b"},
	{"STX %b"},
	{"STA [%b]"},
	{"DEY"},
	{"BIT %a"},
	{"TXA"},
	{"PHB"},
	{"STY %w"},
	{"STA %w"},
	{"STX %w"},
	{"STA %l"},
	{"BCC %r"},
	{"STA (%b),Y"},
	{"STA (%b)"},
	{"STA (%b,S),Y"},
	{"STY %b,X"},
	{"STA %b,X"},
	{"STX %b,Y"},
	{"STA [%b],Y"},
	{"TYA"},
	{"STA %w,Y"},
	{"TXS"},
	{"TXY"},
	{"STZ %w"},
	{"STA %w,X"},
	{"STZ %w,X"},
	{"STA %l,X"},
	{"LDY %i"},
	{"LDA (%b,X)"},
	{"LDX %i"},
	{"LDA %b,S"},
	{"LDY %b"},
	{"LDA %b"},
	{"LDX %b"},
	{"LDA [%b]"},
	{"TAY"},
	{"LDA %a"},
	{"TAX"},
	{"PLB"},
	{"LDY %w"},
	{"LDA %w"},
	{"LDX %w"},
	{"LDA %l"},
	{"BCS %r"},
	{"LDA (%b),Y"},
	{"LDA (%b)"},
	{"LDA (%b,S),Y"},
	{"LDY %b,X"},
	{"LDA %b,X"},
	{"LDX %b,Y"},
	{"LDA [%b],Y"},
	{"CLV"},
	{"LDA %w,Y"},
	{"TSX"},
	{"TYX"},
	{"LDY %w,X"},
	{"LDA %w,X"},
	{"LDX %w,Y"},
	{"LDA %l,X"},
	{"CPY %i"},
	{"CMP (%b,X)"},
	{"REP %f"},
	{"CMP %b"},
	{"CPY %b"},
	{"CMP %b"},
	{"DEC %b"},
	{"CMP [%b]"},
	{"INY"},
	{"CMP %a"},
	{"DEX"},
	{"WAI"},
	{"CPY %w"},
	{"CMP %w"},
	{"DEC %w"},
	{"CMP %l"},
	{"BNE %r"},
	{"CMP (%b),Y"},
	{"CMP (%b)"},
	{"CMP (%b,S),Y"},
	{"PEI (%b)"},
	{"CMP %b,X"},
	{"DEC %b,X"},
	{"CMP [%b],Y"},
	{"CLD"},
	{"CMP %w,Y"},
	{"PHX"},
	{"STP"},
	{"JMP [%j]"},
	{"CMP %w,X"},
	{"DEC %w,X"},
	{"CMP %l,X"},
	{"CPX %i"},
	{"SBC (%b,X)"},
	{"SEP %f"},
	{"SBC %b,S"},
	{"CPX %b"},
	{"SBC %b"},
	{"INC %b"},
	{"SBC [%b]"},
	{"INX"},
	{"SBC %a"},
	{"NOP"},
	{"XBA"},
	{"CPX %w"},
	{"SBC %w"},
	{"INC %w"},
	{"SBC %l"},
	{"BEQ %r"},
	{"SBC (%b),Y"},
	{"SBC (%b)"},
	{"SBC (%b,S),Y"},
	{"PEA %w"},
	{"SBC %b,X"},
	{"INC %b,X"},
	{"SBC [%b],Y"},
	{"SED"},
	{"SBC %w,Y"},
	{"PLX"},
	{"XCE"},
	{"JSR (%j,X)"},
	{"SBC %w,X"},
	{"INC %w,X"},
	{"SBC %l,X"}
};

const QSet<unsigned char> isa_65c816::unlikely = {
        0x00, 0x02, 0x03, 0x13, 0x23, 0x33, 0x42, 0x43, 0x53, 0x63, 
        0x73, 0x83, 0x93, 0xA3, 0xB3, 0xC3, 0xD3, 0xDB, 0xE3, 0xF3, 0xFF
};
