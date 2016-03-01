#include "isa_65c816.h"
#include "utility.h"
#include "debug.h"


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

bool isa_65c816::is_unlikely_opcode(int op)
{
	return error_stop && unlikely.contains(op);
}

bool isa_65c816::is_semiunlikely_opcode(int op)
{
	return error_stop && semiunlikely.contains(op);
}

bool isa_65c816::is_codeflow_opcode(int op)
{
	return codeflow.contains(op);
}

bool isa_65c816::is_stateful_opcode(int op)
{
	if(op == 0x08){
		push_state();
	}else if(op == 0x28){
		pop_state();
	}
	
	return false;
}


bool isa_65c816::is_unlikely_operand()
{
	unsigned int bank = buffer->pc_to_snes(get_base() + delta) & 0xFF0000;
	delta++;
	unsigned int operand_long = get_operand(0) | get_operand(1) | get_operand(2);
	unsigned int operand_word = get_operand(0) | get_operand(1);
	unsigned int operand_byte = get_operand(0);
	unsigned int op = (unsigned char)data.at(delta - 1);
	delta--;
	opcode::operand_hints hint = opcode_list.at(op).hint;
	memory_type type;
	switch(hint){
		case opcode::INDIRECT_JUMP:
		case opcode::WORD_ADDRESS_RAM:
			type = buffer->address_to_type(operand_word | bank);
			if(type != RAM && type != MMIO){
				return true;
			}
		break;
		case opcode::LONG_ADDRESS_RAM:
			type = buffer->address_to_type(operand_long);
			if(type != RAM && type != MMIO){
				return true;
			}
		break;
			
		case opcode::WORD_JUMP:
		case opcode::WORD_ADDRESS_ROM:
			if(buffer->address_to_type(operand_word | bank) != ROM){
				return true;
			}
		break;
			
		case opcode::LONG_JUMP:
			type = buffer->address_to_type(operand_long);
			if(type == UNMAPPED || type == MMIO){
				return true;
			}
			if(type == ROM && buffer->snes_to_pc(operand_long) > buffer->size()){
				return true;
			}
		break;
			
		case opcode::LONG_ADDRESS_ROM:
			if(buffer->address_to_type(operand_long) != ROM){
				return true;
			}
			if(buffer->snes_to_pc(operand_long) > buffer->size()){
				return true;
			}
		break;
			
		case opcode::WORD_ADDRESS_ANY:
			if(buffer->address_to_type(operand_word | bank) == UNMAPPED){
				return true;
			}
		break;
			
		case opcode::LONG_ADDRESS_ANY:
			type = buffer->address_to_type(operand_long);
			if(type == UNMAPPED){
				return true;
			}
			if(type == ROM && buffer->snes_to_pc(operand_long) > buffer->size()){
				return true;
			}
		break;
			
		case opcode::FLAGS:
			if((operand_byte & 0x39) != operand_byte){
				return true;
			}
		break;
			
		case opcode::CONST:
		break;
		case opcode::INDEX:
		break;
			
		case opcode::MOVE:
			if((operand_word & 0x7E00) != 0x7E00 && (operand_word & 0x7E) != 0x7E){
				return true;
			}
		break;
			
		case opcode::BRANCH:
		case opcode::NONE:
		break;
	}
	return false;
}

void isa_65c816::update_state()
{
  	((isa_65c816_ui *)parent)->set_A->setChecked(A_state);
	((isa_65c816_ui *)parent)->set_I->setChecked(I_state);
}

void isa_65c816::set_flags(bookmark_data::types type)
{
	A_state = type & bookmark_data::A;
	I_state = type & bookmark_data::I;
}

void isa_65c816::push_state()
{
	state current_state = {A_state, I_state};
	state_stack.push(current_state);
}

void isa_65c816::pop_state()
{
	if(state_stack.count()){
		state new_state = state_stack.pop();
		A_state = new_state.A_state;
		I_state = new_state.I_state;
	}
}

void isa_65c816::reset_stack()
{
	state_stack.clear();
}

isa_65c816_ui::isa_65c816_ui(QObject *parent) : 
        disassembler_core_ui(parent)
{
	set_disassembler(new isa_65c816(this));
	connect(set_A, &QCheckBox::toggled, this, &isa_65c816_ui::toggle_A);
	connect(set_I, &QCheckBox::toggled, this, &isa_65c816_ui::toggle_I);
	connect(stop, &QCheckBox::toggled, this, &isa_65c816_ui::toggle_error_stop);
}

QGridLayout *isa_65c816_ui::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(set_A, 1, 0, 1, 1);
	grid->addWidget(set_I, 2, 0, 1, 1);
	grid->addWidget(stop, 1, 1, 2, 1);
	return grid;
}


isa_65c816_ui::~isa_65c816_ui()
{
	delete stop;
	delete set_A;
	delete set_I;
}
using hint = disassembler_core::opcode;
const QList<disassembler_core::opcode> isa_65c816::opcode_list = {
        {"BRK %c",	hint::NONE},
	{"ORA (%b,X)",	hint::NONE},
	{"COP %c",	hint::NONE},
	{"ORA %b,s",	hint::NONE},
	{"TSB %b",	hint::NONE},
	{"ORA %b",	hint::NONE},
	{"ASL %b",	hint::NONE},
	{"ORA [%b]",	hint::NONE},
	{"PHP",		hint::NONE},
	{"ORA %a",	hint::CONST},
	{"ASL A",	hint::NONE},
	{"PHD",		hint::NONE},
	{"TSB %w",	hint::WORD_ADDRESS_RAM},
	{"ORA %w",	hint::WORD_ADDRESS_RAM},
	{"ASL %w",	hint::WORD_ADDRESS_RAM},
	{"ORA %l",	hint::LONG_ADDRESS_RAM},
	{"BPL %r",	hint::BRANCH},
	{"ORA (%b),Y",	hint::NONE},
	{"ORA (%b)",	hint::NONE},
	{"ORA (%b,S),Y",hint::NONE},
	{"TRB %b",	hint::NONE},
	{"ORA %b,X",	hint::NONE},
	{"ASL %b,X",	hint::NONE},
	{"ORA [%b],Y",	hint::NONE},
	{"CLC",		hint::NONE},
	{"ORA %w,Y",	hint::WORD_ADDRESS_ANY},
	{"INC A",	hint::NONE},
	{"TCS",		hint::NONE},
	{"TRB %w",	hint::WORD_ADDRESS_RAM},
	{"ORA %w,X",	hint::WORD_ADDRESS_ANY},
	{"ASL %w,X",	hint::WORD_ADDRESS_RAM},
	{"ORA %l,X",	hint::LONG_ADDRESS_ANY},
	{"JSR %j",	hint::WORD_JUMP},
	{"AND (%b,X)",	hint::NONE},
	{"JSL %J",	hint::LONG_JUMP},
	{"AND %b",	hint::NONE},
	{"BIT %b",	hint::NONE},
	{"AND %b",	hint::NONE},
	{"ROL %b",	hint::NONE},
	{"AND [%b]",	hint::NONE},
	{"PLP",		hint::NONE},
	{"AND %a",	hint::CONST},
	{"ROL A",	hint::NONE},
	{"PLD",		hint::NONE},
	{"BIT %w",	hint::WORD_ADDRESS_RAM},
	{"AND %w",	hint::WORD_ADDRESS_RAM},
	{"ROL %w",	hint::WORD_ADDRESS_RAM},
	{"AND %l",	hint::LONG_ADDRESS_RAM},
	{"BMI %r",	hint::BRANCH},
	{"AND (%b),Y",	hint::NONE},
	{"AND (%b)",	hint::NONE},
	{"AND (%b,S),Y",hint::NONE},
	{"BIT %b,X",	hint::NONE},
	{"AND %b,X",	hint::NONE},
	{"ROL %b,X",	hint::NONE},
	{"AND [%b],Y",	hint::NONE},
	{"SEC",		hint::NONE},
	{"AND %w,Y",	hint::WORD_ADDRESS_ANY},
	{"DEC A",	hint::NONE},
	{"TSC",		hint::NONE},
	{"BIT %b,X",	hint::NONE},
	{"AND %b,X",	hint::NONE},
	{"ROL %b,X",	hint::NONE},
	{"AND %l,X",	hint::LONG_ADDRESS_RAM},
	{"RTI",		hint::NONE},
	{"EOR (%b,X)",	hint::NONE},
	{"WDM %b",	hint::NONE},
	{"EOR %b,s",	hint::NONE},
	{"MVP %w",	hint::MOVE},
	{"EOR %b",	hint::NONE},
	{"LSR %b",	hint::NONE},
	{"EOR [%b]",	hint::NONE},
	{"PHA",		hint::NONE},
	{"EOR %a",	hint::CONST},
	{"LSR A",	hint::NONE},
	{"PHK",		hint::NONE},
	{"JMP %j",	hint::WORD_JUMP},
	{"EOR %w",	hint::WORD_ADDRESS_RAM},
	{"LSR %w",	hint::WORD_ADDRESS_RAM},
	{"EOR %l",	hint::LONG_ADDRESS_RAM},
	{"BVC %r",	hint::BRANCH},
	{"EOR (%b),Y",	hint::NONE},
	{"EOR (%b)",	hint::NONE},
	{"EOR (%b,S),Y",hint::NONE},
	{"MVN %w",	hint::MOVE},
	{"EOR %b,X",	hint::NONE},
	{"LSR %b,X",	hint::NONE},
	{"EOR [%b],Y",	hint::NONE},
	{"CLI",		hint::NONE},
	{"EOR %w,Y",	hint::WORD_ADDRESS_ANY},
	{"PHY",		hint::NONE},
	{"TCD",		hint::NONE},
	{"JML %J",	hint::LONG_JUMP},
	{"EOR %w,X",	hint::WORD_ADDRESS_ANY},
	{"LSR %w,X",	hint::WORD_ADDRESS_RAM},
	{"EOR %l,X",	hint::LONG_ADDRESS_ANY},
	{"RTS",		hint::NONE},
	{"ADC (%b,X)",	hint::NONE},
	{"PER %w",	hint::NONE},
	{"ADC %b,s",	hint::NONE},
	{"STZ %b",	hint::NONE},
	{"ADC %b",	hint::NONE},
	{"ROR %b",	hint::NONE},
	{"ADC [%b]",	hint::NONE},
	{"PLA",		hint::NONE},
	{"ADC %a",	hint::CONST},
	{"ROR A",	hint::NONE},
	{"RTL",		hint::NONE},
	{"JMP (%j)",	hint::NONE},
	{"ADC %w",	hint::WORD_ADDRESS_RAM},
	{"ROR %w",	hint::WORD_ADDRESS_RAM},
	{"ADC %l",	hint::LONG_ADDRESS_RAM},
	{"BVS %r",	hint::BRANCH},
	{"ADC (%b),Y",	hint::NONE},
	{"ADC (%b)",	hint::NONE},
	{"ADC (%b,S),Y",hint::NONE},
	{"STZ %b,X",	hint::NONE},
	{"ADC %b,X",	hint::NONE},
	{"ROR %b,X",	hint::NONE},
	{"ADC [%b],Y",	hint::NONE},
	{"SEI",		hint::NONE},
	{"ADC %w,Y",	hint::WORD_ADDRESS_ANY},
	{"PLY",		hint::NONE},
	{"TDC",		hint::NONE},
	{"JMP (%j,X)",	hint::NONE},
	{"ADC %w,X",	hint::WORD_ADDRESS_ANY},
	{"ROR %w,X",	hint::WORD_ADDRESS_RAM},
	{"ADC %l,X",	hint::LONG_ADDRESS_ANY},
	{"BRA %r",	hint::BRANCH},
	{"STA (%b,X)",	hint::NONE},
	{"BRL %r",	hint::BRANCH},
	{"STA %b,s",	hint::NONE},
	{"STY %b",	hint::NONE},
	{"STA %b",	hint::NONE},
	{"STX %b",	hint::NONE},
	{"STA [%b]",	hint::NONE},
	{"DEY",		hint::NONE},
	{"BIT %a",	hint::CONST},
	{"TXA",		hint::NONE},
	{"PHB",		hint::NONE},
	{"STY %w",	hint::WORD_ADDRESS_RAM},
	{"STA %w",	hint::WORD_ADDRESS_RAM},
	{"STX %w",	hint::WORD_ADDRESS_RAM},
	{"STA %l",	hint::LONG_ADDRESS_RAM},
	{"BCC %r",	hint::BRANCH},
	{"STA (%b),Y",	hint::NONE},
	{"STA (%b)",	hint::NONE},
	{"STA (%b,S),Y",hint::NONE},
	{"STY %b,X",	hint::NONE},
	{"STA %b,X",	hint::NONE},
	{"STX %b,Y",	hint::NONE},
	{"STA [%b],Y",	hint::NONE},
	{"TYA",		hint::NONE},
	{"STA %w,Y",	hint::WORD_ADDRESS_RAM},
	{"TXS",		hint::NONE},
	{"TXY",		hint::NONE},
	{"STZ %w",	hint::WORD_ADDRESS_RAM},
	{"STA %w,X",	hint::WORD_ADDRESS_RAM},
	{"STZ %w,X",	hint::WORD_ADDRESS_RAM},
	{"STA %l,X",	hint::LONG_ADDRESS_ANY},
	{"LDY %i",	hint::INDEX},
	{"LDA (%b,X)",	hint::NONE},
	{"LDX %i",	hint::INDEX},
	{"LDA %b,s",	hint::NONE},
	{"LDY %b",	hint::NONE},
	{"LDA %b",	hint::NONE},
	{"LDX %b",	hint::NONE},
	{"LDA [%b]",	hint::NONE},
	{"TAY",		hint::NONE},
	{"LDA %a",	hint::CONST},
	{"TAX",		hint::NONE},
	{"PLB",		hint::NONE},
	{"LDY %w",	hint::WORD_ADDRESS_ANY},
	{"LDA %w",	hint::WORD_ADDRESS_ANY},
	{"LDX %w",	hint::WORD_ADDRESS_ANY},
	{"LDA %l",	hint::LONG_ADDRESS_ANY},
	{"BCS %r",	hint::BRANCH},
	{"LDA (%b),Y",	hint::NONE},
	{"LDA (%b)",	hint::NONE},
	{"LDA (%b,S),Y",hint::NONE},
	{"LDY %b,X",	hint::NONE},
	{"LDA %b,X",	hint::NONE},
	{"LDX %b,Y",	hint::NONE},
	{"LDA [%b],Y",	hint::NONE},
	{"CLV",		hint::NONE},
	{"LDA %w,Y",	hint::WORD_ADDRESS_ANY},
	{"TSX",		hint::NONE},
	{"TYX",		hint::NONE},
	{"LDY %w,X",	hint::WORD_ADDRESS_ANY},
	{"LDA %w,X",	hint::WORD_ADDRESS_ANY},
	{"LDX %w,Y",	hint::WORD_ADDRESS_ANY},
	{"LDA %l,X",	hint::LONG_ADDRESS_ANY},
	{"CPY %i",	hint::INDEX},
	{"CMP (%b,X)",	hint::NONE},
	{"REP %f",	hint::FLAGS},
	{"CMP %b",	hint::NONE},
	{"CPY %b",	hint::NONE},
	{"CMP %b",	hint::NONE},
	{"DEC %b",	hint::NONE},
	{"CMP [%b]",	hint::NONE},
	{"INY",		hint::NONE},
	{"CMP %a",	hint::CONST},
	{"DEX",		hint::NONE},
	{"WAI",		hint::NONE},
	{"CPY %w",	hint::WORD_ADDRESS_RAM},
	{"CMP %w",	hint::WORD_ADDRESS_RAM},
	{"DEC %w",	hint::WORD_ADDRESS_RAM},
	{"CMP %l",	hint::LONG_ADDRESS_RAM},
	{"BNE %r",	hint::BRANCH},
	{"CMP (%b),Y",	hint::NONE},
	{"CMP (%b)",	hint::NONE},
	{"CMP (%b,S),Y",hint::NONE},
	{"PEI (%b)",	hint::NONE},
	{"CMP %b,X",	hint::NONE},
	{"DEC %b,X",	hint::NONE},
	{"CMP [%b],Y",	hint::NONE},
	{"CLD",		hint::NONE},
	{"CMP %w,Y",	hint::WORD_ADDRESS_RAM},
	{"PHX",		hint::NONE},
	{"STP",		hint::NONE},
	{"JMP [%j]",	hint::INDIRECT_JUMP},
	{"CMP %w,X",	hint::WORD_ADDRESS_RAM},
	{"DEC %w,X",	hint::WORD_ADDRESS_RAM},
	{"CMP %l,X",	hint::LONG_ADDRESS_RAM},
	{"CPX %i",	hint::INDEX},
	{"SBC (%b,X)",	hint::NONE},
	{"SEP %f",	hint::FLAGS},
	{"SBC %b,s",	hint::NONE},
	{"CPX %b",	hint::NONE},
	{"SBC %b",	hint::NONE},
	{"INC %b",	hint::NONE},
	{"SBC [%b]",	hint::NONE},
	{"INX",		hint::NONE},
	{"SBC %a",	hint::CONST},
	{"NOP",		hint::NONE},
	{"XBA",		hint::NONE},
	{"CPX %w",	hint::WORD_ADDRESS_RAM},
	{"SBC %w",	hint::WORD_ADDRESS_RAM},
	{"INC %w",	hint::WORD_ADDRESS_RAM},
	{"SBC %l",	hint::LONG_ADDRESS_RAM},
	{"BEQ %r",	hint::BRANCH},
	{"SBC (%b),Y",	hint::NONE},
	{"SBC (%b)",	hint::NONE},
	{"SBC (%b,S),Y",hint::NONE},
	{"PEA %w",	hint::NONE},
	{"SBC %b,X",	hint::NONE},
	{"INC %b,X",	hint::NONE},
	{"SBC [%b],Y",	hint::NONE},
	{"SED",		hint::NONE},
	{"SBC %w,Y",	hint::WORD_ADDRESS_ANY},
	{"PLX",		hint::NONE},
	{"XCE",		hint::NONE},
	{"JSR (%j,X)",	hint::NONE},
	{"SBC %w,X",	hint::WORD_ADDRESS_ANY},
	{"INC %w,X",	hint::WORD_ADDRESS_RAM},
	{"SBC %l,X",	hint::LONG_ADDRESS_ANY}
};

const QSet<unsigned char> isa_65c816::unlikely = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x13, 0x23, 0x33, 0x42, 0x43, 0x53, 0x63, 
        0x73, 0x83, 0x93, 0xA3, 0xB3, 0xC3, 0xD3, 0xDB, 0xE3, 0xF3, 0xFF
};

const QSet<unsigned char> isa_65c816::semiunlikely = {
	0x01, 0x04, 0x07, 0x0B, 0x0F, 0x11, 0x12, 0x14, 0x17, 0x1B, 0x1C, 0x1F, 0x21, 0x27, 0x2B, 0x2F,
	0x30, 0x31, 0x32, 0x37, 0x3B, 0x3F, 0x40, 0x41, 0x44, 0x47, 0x4F, 0x50, 0x51, 0x52, 0x54, 0x57,
	0x58, 0x5B, 0x5F, 0x61, 0x62, 0x67, 0x68, 0x6F, 0x70, 0x71, 0x72, 0x76, 0x77, 0x7A, 0x7B,
	0x7F, 0x81, 0x82, 0x87, 0x8F, 0x91, 0x92, 0x9A, 0xA1, 0xA7, 0xAB, 0xAF, 0xB1, 0xB2,
	0xB8, 0xBA, 0xC1, 0xC7, 0xCB, 0xCF, 0xD1, 0xD2, 0xD4, 0xD7, 0xD8, 0xE1, 0xE7, 0xEA, 0xEB, 0xEF,
	0xF1, 0xF2, 0xF4, 0xF7, 0xF8, 0xFA, 0xFB, 0xFE
};

const QSet<unsigned char> isa_65c816::codeflow = {
	0x4C, 0x5C, 0x60, 0x6B, 0x6C, 0x7C, 0x80, 0xDC
};
