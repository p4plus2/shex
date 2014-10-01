#include "isa_65c816.h"
#include "../debug.h"

isa_65c816::isa_65c816(QObject *parent) :
        disassembler_core(parent)
{
	connect(this, &isa_65c816::A_changed, set_A, &QCheckBox::setChecked);
	connect(this, &isa_65c816::I_changed, set_I, &QCheckBox::setChecked);
			
	connect(set_A, &QCheckBox::toggled, this, &isa_65c816::toggle_A);
	connect(set_I, &QCheckBox::toggled, this, &isa_65c816::toggle_I);
	connect(stop, &QCheckBox::toggled, this, &isa_65c816::toggle_error_stop);
}
#define get_operand(B) ((size > B && (i+B) < data.size()) ? ((unsigned char)data.at(i+B) << (B-1)*8) : 0)
#define hex_to_string(A) QString::number(operand, 16).rightJustified((A), '0').toUpper()
#define label_op(T) \
	QByteArray little_endian = buffer->to_little_endian(data.mid(i+1, size-1)); \
	int destination = buffer->snes_to_pc(buffer->T##_address(i+start, little_endian)); \
	if(destination < start || destination > end){ \
		QString op_text = hex_to_string((size-1)*2); \
		add_mnemonic(start+i, op.name.arg('$' + op_text)); \
	}else{ \
		add_mnemonic(start+i, op.name.arg(add_label(destination))); \
	}

QString isa_65c816::disassemble(int start, int end, const ROM_buffer *buffer)
{
	reset();
	QByteArray data = buffer->range(start, end);
	bool stopped = false;
	for(int i = 0; i < data.size();){
		unsigned char hex = data.at(i);
		disassembler_core::opcode op = opcode_list[hex];
		int size = op.size;
		
		if(error_stop && unlikely.contains(hex)){
			stopped = true;
			break;
		}else if(op.size > 1){
			size += (A_state && A_16_list.contains(hex)) || (I_state && I_16_list.contains(hex));
			int operand = get_operand(1) | get_operand(2) | get_operand(3);
			if(i+size > data.size()){
				QString op_text = hex_to_string(i+size-data.size()*2);
				if(data.size() - i == 1){
					op_text.clear();
				}
				QString dollar = (branch_list.contains(hex) || jump_list.contains(hex)) ? "$" : "";
				add_mnemonic(start+i, op.name.arg(dollar + op_text.rightJustified((size-1)*2, 'X')));
			}else if(branch_list.contains(hex)){
				label_op(branch);
			}else if(jump_list.contains(hex)){
				label_op(jump);
			}else{
				QString op_text = hex_to_string((size-1)*2);
				add_mnemonic(start+i, op.name.arg(op_text));
				if(hex == 0xC2 || hex == 0xE2){
					A_state = (operand & 0x20) ? hex == 0xC2 : A_state;
					I_state = (operand & 0x10) ? hex == 0xC2 : I_state;			
				}
			}
		}else{
			add_mnemonic(start+i, op.name);
		}
		i += size;
	}
	emit A_changed(A_state);
	emit I_changed(I_state);
	return disassembly_text() + (stopped ? "Unlikely opcode detected, aborting!" : "");
}

#undef label_op
#undef hex_to_string
#undef get_operand

QGridLayout *isa_65c816::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(set_A, 1, 0, 1, 1);
	grid->addWidget(set_I, 2, 0, 1, 1);
	grid->addWidget(stop, 1, 1, 2, 1);
	return grid;
}



const QList<disassembler_core::opcode> isa_65c816::opcode_list = {
	{"BRK #$%1", 2},
	{"ORA ($%1,X)", 2},
	{"COP #$%1", 2},
	{"ORA $%1,S", 2},
	{"TSB $%1", 2},
	{"ORA $%1", 2},
	{"ASL $%1", 2},
	{"ORA [$%1]", 2},
	{"PHP", 1},
	{"ORA #$%1", 2},
	{"ASL A", 1},
	{"PHD", 1},
	{"TSB $%1", 3},
	{"ORA $%1", 3},
	{"ASL $%1", 3},
	{"ORA $%1", 4},
	{"BPL %1", 2},
	{"ORA ($%1),Y", 2},
	{"ORA ($%1)", 2},
	{"ORA ($%1,S),Y", 2},
	{"TRB $%1", 2},
	{"ORA $%1,X", 2},
	{"ASL $%1,X", 2},
	{"ORA [$%1],Y", 2},
	{"CLC", 1},
	{"ORA $%1,Y", 3},
	{"INC A", 1},
	{"TCS", 1},
	{"TRB $%1", 3},
	{"ORA $%1,X", 3},
	{"ASL $%1,X", 3},
	{"ORA $%1,X", 4},
	{"JSR %1", 3},
	{"AND ($%1,X)", 2},
	{"JSL %1", 4},
	{"AND $%1,S", 2},
	{"BIT $%1", 2},
	{"AND $%1", 2},
	{"ROL $%1", 2},
	{"AND [$%1]", 2},
	{"PLP", 1},
	{"AND #$%1", 2},
	{"ROL A", 1},
	{"PLD", 1},
	{"BIT $%1", 3},
	{"AND $%1", 3},
	{"ROL $%1", 3},
	{"AND $%1", 4},
	{"BMI %1", 2},
	{"AND ($%1),Y", 2},
	{"AND ($%1)", 2},
	{"AND ($%1,S),Y", 2},
	{"BIT $%1,X", 2},
	{"AND $%1,X", 2},
	{"ROL $%1,X", 2},
	{"AND [$%1],Y", 2},
	{"SEC", 1},
	{"AND $%1,Y", 3},
	{"DEC A", 1},
	{"TSC", 1},
	{"BIT $%1,X", 2},
	{"AND $%1,X", 2},
	{"ROL $%1,X", 2},
	{"AND $%1,X", 4},
	{"RTI", 1},
	{"EOR ($%1,X)", 2},
	{"WDM $%1", 2},
	{"EOR $%1,S", 2},
	{"MVN $%1", 3},
	{"EOR $%1", 2},
	{"LSR $%1", 2},
	{"EOR [$%1]", 2},
	{"PHA", 1},
	{"EOR #$%1", 2},
	{"LSR A", 1},
	{"PHK", 1},
	{"JMP %1", 3},
	{"EOR $%1", 3},
	{"LSR $%1", 3},
	{"EOR $%1", 4},
	{"BVC %1", 2},
	{"EOR ($%1),Y", 2},
	{"EOR ($%1)", 2},
	{"EOR ($%1,S),Y", 2},
	{"MVN $%1", 3},
	{"EOR $%1,X", 2},
	{"LSR $%1,X", 2},
	{"EOR [$%1],Y", 2},
	{"CLI", 1},
	{"EOR $%1,Y", 3},
	{"PHY", 1},
	{"TCD", 1},
	{"JML %1", 4},
	{"EOR $%1,X", 3},
	{"LSR $%1,X", 3},
	{"EOR $%1,X", 4},
	{"RTS", 1},
	{"ADC ($%1,X)", 2},
	{"PER $%1", 3},
	{"ADC $%1,S", 2},
	{"STZ $%1", 2},
	{"ADC $%1", 2},
	{"ROR $%1", 2},
	{"ADC [$%1]", 2},
	{"PLA", 1},
	{"ADC #$%1", 2},
	{"ROR A", 1},
	{"RTL", 1},
	{"JMP (%1)", 3},
	{"ADC $%1", 3},
	{"ROR $%1", 3},
	{"ADC $%1", 4},
	{"BVS %1", 2},
	{"ADC ($%1),Y", 2},
	{"ADC ($%1)", 2},
	{"ADC ($%1,S),Y", 2},
	{"STZ $%1,X", 2},
	{"ADC $%1,X", 2},
	{"ROR $%1,X", 2},
	{"ADC [$%1],Y", 2},
	{"SEI", 1},
	{"ADC $%1,Y", 3},
	{"PLY", 1},
	{"TDC", 1},
	{"JMP (%1,X)", 3},
	{"ADC $%1,X", 3},
	{"ROR $%1,X", 3},
	{"ADC $%1,X", 4},
	{"BRA %1", 2},
	{"STA ($%1,X)", 2},
	{"BRL %1", 3},
	{"STA $%1,S", 2},
	{"STY $%1", 2},
	{"STA $%1", 2},
	{"STX $%1", 2},
	{"STA [$%1]", 2},
	{"DEY", 1},
	{"BIT #$%1", 2},
	{"TXA", 1},
	{"PHB", 1},
	{"STY $%1", 3},
	{"STA $%1", 3},
	{"STX $%1", 3},
	{"STA $%1", 4},
	{"BCC %1", 2},
	{"STA ($%1),Y", 2},
	{"STA ($%1)", 2},
	{"STA ($%1,S),Y", 2},
	{"STY $%1,X", 2},
	{"STA $%1,X", 2},
	{"STX $%1,Y", 2},
	{"STA [$%1],Y", 2},
	{"TYA", 1},
	{"STA $%1,Y", 3},
	{"TXS", 1},
	{"TXY", 1},
	{"STZ $%1", 3},
	{"STA $%1,X", 3},
	{"STZ $%1,X", 3},
	{"STA $%1,X", 4},
	{"LDY #$%1", 2},
	{"LDA ($%1,X)", 2},
	{"LDX #$%1", 2},
	{"LDA $%1,S", 2},
	{"LDY $%1", 2},
	{"LDA $%1", 2},
	{"LDX $%1", 2},
	{"LDA [$%1]", 2},
	{"TAY", 1},
	{"LDA #$%1", 2},
	{"TAX", 1},
	{"PLB", 1},
	{"LDY $%1", 3},
	{"LDA $%1", 3},
	{"LDX $%1", 3},
	{"LDA $%1", 4},
	{"BCS %1", 2},
	{"LDA ($%1),Y", 2},
	{"LDA ($%1)", 2},
	{"LDA ($%1,S),Y", 2},
	{"LDY $%1,X", 2},
	{"LDA $%1,X", 2},
	{"LDX $%1,Y", 2},
	{"LDA [$%1],Y", 2},
	{"CLV", 1},
	{"LDA $%1,Y", 3},
	{"TSX", 1},
	{"TYX", 1},
	{"LDY $%1,X", 3},
	{"LDA $%1,X", 3},
	{"LDX $%1,Y", 3},
	{"LDA $%1,X", 4},
	{"CPY #$%1", 2},
	{"CMP ($%1,X)", 2},
	{"REP #$%1", 2},
	{"CMP $%1,S", 2},
	{"CPY $%1", 2},
	{"CMP $%1", 2},
	{"DEC $%1", 2},
	{"CMP [$%1]", 2},
	{"INY", 1},
	{"CMP #$%1", 2},
	{"DEX", 1},
	{"WAI", 1},
	{"CPY $%1", 3},
	{"CMP $%1", 3},
	{"DEC $%1", 3},
	{"CMP $%1", 4},
	{"BNE %1", 2},
	{"CMP ($%1),Y", 2},
	{"CMP ($%1)", 2},
	{"CMP ($%1,S),Y", 2},
	{"PEI ($%1)", 2},
	{"CMP $%1,X", 2},
	{"DEC $%1,X", 2},
	{"CMP [$%1],Y", 2},
	{"CLD", 1},
	{"CMP $%1,Y", 3},
	{"PHX", 1},
	{"STP", 1},
	{"JMP [%1]", 3},
	{"CMP $%1,X", 3},
	{"DEC $%1,X", 3},
	{"CMP $%1,X", 4},
	{"CPX #$%1", 2},
	{"SBC ($%1,X)", 2},
	{"SEP #$%1", 2},
	{"SBC $%1,S", 2},
	{"CPX $%1", 2},
	{"SBC $%1", 2},
	{"INC $%1", 2},
	{"SBC [$%1]", 2},
	{"INX", 1},
	{"SBC #$%1", 2},
	{"NOP", 1},
	{"XBA", 1},
	{"CPX $%1", 3},
	{"SBC $%1", 3},
	{"INC $%1", 3},
	{"SBC $%1", 4},
	{"BEQ %1", 2},
	{"SBC ($%1),Y", 2},
	{"SBC ($%1)", 2},
	{"SBC ($%1,S),Y", 2},
	{"PEA $%1", 3},
	{"SBC $%1,X", 2},
	{"INC $%1,X", 2},
	{"SBC [$%1],Y", 2},
	{"SED", 1},
	{"SBC $%1,Y", 3},
	{"PLX", 1},
	{"XCE", 1},
	{"JSR (%1,X)", 3},
	{"SBC $%1,X", 3},
	{"INC $%1,X", 3},
	{"SBC $%1,X", 4}
};

const QSet<unsigned char> isa_65c816::A_16_list = {
	0x09, 0x29, 0x49, 0x69, 0x89, 0xA9, 0xC9, 0xE9
};

const QSet<unsigned char> isa_65c816::I_16_list = {
	0xA0, 0xA2, 0xC0, 0xE0
};

const QSet<unsigned char> isa_65c816::unlikely = {
        0x00, 0x02, 0x03, 0x13, 0x23, 0x33, 0x42, 0x43, 0x53, 0x63, 
        0x73, 0x83, 0x93, 0xA3, 0xB3, 0xC3, 0xD3, 0xDB, 0xE3, 0xF3, 0xFF
};

const QSet<unsigned char> isa_65c816::branch_list = {
	0x10, 0x30, 0x50, 0x70, 0x80, 0x82, 0x90, 0xB0, 0xD0, 0xF0
};

const QSet<unsigned char> isa_65c816::jump_list = {
	0x20, 0x22, 0x4C, 0x5C, 0x6C, 0x7C, 0xDC, 0xFC
};
