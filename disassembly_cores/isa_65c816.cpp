#include "isa_65c816.h"

isa_65c816::isa_65c816(QObject *parent) :
        disassembler_core(parent)
{
	connect(this, SIGNAL(A_changed(bool)), set_A, SLOT(setChecked(bool)));
	connect(this, SIGNAL(I_changed(bool)), set_I, SLOT(setChecked(bool)));
			
	connect(set_A, SIGNAL(toggled(bool)), this, SLOT(toggle_A(bool)));
	connect(set_I, SIGNAL(toggled(bool)), this, SLOT(toggle_I(bool)));
	connect(stop, SIGNAL(toggled(bool)), this, SLOT(toggle_error_stop(bool)));
}

QString isa_65c816::disassemble(QByteArray *data)
{
	QString decoded;
	for(int i = 0; i < data->size();){
		unsigned char hex = (unsigned char)data->at(i);
		disassembler_core::opcode op = opcode_list[hex];
		int size = op.size;
		if(op.size > 1){
			size += (A_state && A_16_list.contains(hex)) || (I_state && I_16_list.contains(hex));
			int operand = (size > 1 && (i+1) < data->size()) ? (unsigned char)data->at(i+1) : 0;
			operand |= (size > 2 && (i+2) < data->size()) ? ((unsigned char)data->at(i+2) << 8) : 0;
			operand |= (size > 3 && (i+3) < data->size()) ? ((unsigned char)data->at(i+3) << 16) : 0;
			if(i+size > data->size()){
				QString hex = QString::number(operand, 16).rightJustified((i+size-data->size())*2, '0');
				decoded.append(op.name.arg(hex.rightJustified((size-1)*2, 'X')));
			}else{
				decoded.append(op.name.arg(QString::number(operand, 16).rightJustified((size-1)*2, '0')));
			}
			if(hex == 0xC2){
				A_state = (data->at(i+1) & 0x20) ? 1 : 0;
				I_state = (data->at(i+1) & 0x10) ? 1 : 0;			
			}else if(hex == 0xE2){
				A_state = (data->at(i+1) & 0x20) ? 0 : 1;
				I_state = (data->at(i+1) & 0x10) ? 0 : 1;
			}
		}else{
			decoded.append(op.name);
		}
		if(error_stop && unlikely.contains(hex)){
			decoded.append("Unlikely opcode detected, aborting!");
			break;
		}
		i += size;
	}
	emit A_changed(A_state);
	emit I_changed(I_state);
	return decoded;
}

QGridLayout *isa_65c816::core_layout()
{
	QGridLayout *grid = new QGridLayout();
	grid->addWidget(set_A, 1, 0, 1, 1);
	grid->addWidget(set_I, 2, 0, 1, 1);
	grid->addWidget(stop, 1, 1, 2, 1);
	return grid;
}



const QList<disassembler_core::opcode> isa_65c816::opcode_list = {
	{"BRK #$%1\n", 2},
	{"ORA ($%1,X)\n", 2},
	{"COP #$%1\n", 2},
	{"ORA $%1,S\n", 2},
	{"TSB $%1\n", 2},
	{"ORA $%1\n", 2},
	{"ASL $%1\n", 2},
	{"ORA [$%1]\n", 2},
	{"PHP\n", 1},
	{"ORA #$%1\n", 2},
	{"ASL A\n", 1},
	{"PHD\n", 1},
	{"TSB $%1\n", 3},
	{"ORA $%1\n", 3},
	{"ASL $%1\n", 3},
	{"ORA $%1\n", 4},
	{"BPL $%1\n", 2},
	{"ORA ($%1),Y\n", 2},
	{"ORA ($%1)\n", 2},
	{"ORA ($%1,S),Y\n", 2},
	{"TRB $%1\n", 2},
	{"ORA $%1,X\n", 2},
	{"ASL $%1,X\n", 2},
	{"ORA [$%1],Y\n", 2},
	{"CLC\n", 1},
	{"ORA $%1,Y\n", 3},
	{"INC A\n", 1},
	{"TCS\n", 1},
	{"TRB $%1\n", 3},
	{"ORA $%1,X\n", 3},
	{"ASL $%1,X\n", 3},
	{"ORA $%1,X\n", 4},
	{"JSR $%1\n", 3},
	{"AND ($%1,X)\n", 2},
	{"JSL $%1\n", 4},
	{"AND $%1,S\n", 2},
	{"BIT $%1\n", 2},
	{"AND $%1\n", 2},
	{"ROL $%1\n", 2},
	{"AND [$%1]\n", 2},
	{"PLP\n", 1},
	{"AND #$%1\n", 2},
	{"ROL A\n", 1},
	{"PLD\n", 1},
	{"BIT $%1\n", 3},
	{"AND $%1\n", 3},
	{"ROL $%1\n", 3},
	{"AND $%1\n", 4},
	{"BMI $%1\n", 2},
	{"AND ($%1),Y\n", 2},
	{"AND ($%1)\n", 2},
	{"AND ($%1,S),Y\n", 2},
	{"BIT $%1,X\n", 2},
	{"AND $%1,X\n", 2},
	{"ROL $%1,X\n", 2},
	{"AND [$%1],Y\n", 2},
	{"SEC\n", 1},
	{"AND $%1,Y\n", 3},
	{"DEC A\n", 1},
	{"TSC\n", 1},
	{"BIT $%1,X\n", 2},
	{"AND $%1,X\n", 2},
	{"ROL $%1,X\n", 2},
	{"AND $%1,X\n", 4},
	{"RTI\n", 1},
	{"EOR ($%1,X)\n", 2},
	{"WDM $%1\n", 2},
	{"EOR $%1,S\n", 2},
	{"MVN $%1\n", 3},
	{"EOR $%1\n", 2},
	{"LSR $%1\n", 2},
	{"EOR [$%1]\n", 2},
	{"PHA\n", 1},
	{"EOR #$%1\n", 2},
	{"LSR A\n", 1},
	{"PHK\n", 1},
	{"JMP $%1\n", 3},
	{"EOR $%1\n", 3},
	{"LSR $%1\n", 3},
	{"EOR $%1\n", 4},
	{"BVC $%1\n", 2},
	{"EOR ($%1),Y\n", 2},
	{"EOR ($%1)\n", 2},
	{"EOR ($%1,S),Y\n", 2},
	{"MVN $%1\n", 3},
	{"EOR $%1,X\n", 2},
	{"LSR $%1,X\n", 2},
	{"EOR [$%1],Y\n", 2},
	{"CLI\n", 1},
	{"EOR $%1,Y\n", 3},
	{"PHY\n", 1},
	{"TCD\n", 1},
	{"JML $%1\n", 4},
	{"EOR $%1,X\n", 3},
	{"LSR $%1,X\n", 3},
	{"EOR $%1,X\n", 4},
	{"RTS\n", 1},
	{"ADC ($%1,X)\n", 2},
	{"PER $%1\n", 3},
	{"ADC $%1,S\n", 2},
	{"STZ $%1\n", 2},
	{"ADC $%1\n", 2},
	{"ROR $%1\n", 2},
	{"ADC [$%1]\n", 2},
	{"PLA\n", 1},
	{"ADC #$%1\n", 2},
	{"ROR A\n", 1},
	{"RTL\n", 1},
	{"JMP ($%1)\n", 3},
	{"ADC $%1\n", 3},
	{"ROR $%1\n", 3},
	{"ADC $%1\n", 4},
	{"BVS $%1\n", 2},
	{"ADC ($%1),Y\n", 2},
	{"ADC ($%1)\n", 2},
	{"ADC ($%1,S),Y\n", 2},
	{"STZ $%1,X\n", 2},
	{"ADC $%1,X\n", 2},
	{"ROR $%1,X\n", 2},
	{"ADC [$%1],Y\n", 2},
	{"SEI\n", 1},
	{"ADC $%1,Y\n", 3},
	{"PLY\n", 1},
	{"TDC\n", 1},
	{"JMP ($%1,X)\n", 3},
	{"ADC $%1,X\n", 3},
	{"ROR $%1,X\n", 3},
	{"ADC $%1,X\n", 4},
	{"BRA $%1\n", 2},
	{"STA ($%1,X)\n", 2},
	{"BRL $%1\n", 3},
	{"STA $%1,S\n", 2},
	{"STY $%1\n", 2},
	{"STA $%1\n", 2},
	{"STX $%1\n", 2},
	{"STA [$%1]\n", 2},
	{"DEY\n", 1},
	{"BIT #$%1\n", 2},
	{"TXA\n", 1},
	{"PHB\n", 1},
	{"STY $%1\n", 3},
	{"STA $%1\n", 3},
	{"STX $%1\n", 3},
	{"STA $%1\n", 4},
	{"BCC $%1\n", 2},
	{"STA ($%1),Y\n", 2},
	{"STA ($%1)\n", 2},
	{"STA ($%1,S),Y\n", 2},
	{"STY $%1,X\n", 2},
	{"STA $%1,X\n", 2},
	{"STX $%1,Y\n", 2},
	{"STA [$%1],Y\n", 2},
	{"TYA\n", 1},
	{"STA $%1,Y\n", 3},
	{"TXS\n", 1},
	{"TXY\n", 1},
	{"STZ $%1\n", 3},
	{"STA $%1,X\n", 3},
	{"STZ $%1,X\n", 3},
	{"STA $%1,X\n", 4},
	{"LDY #$%1\n", 2},
	{"LDA ($%1,X)\n", 2},
	{"LDX #$%1\n", 2},
	{"LDA $%1,S\n", 2},
	{"LDY $%1\n", 2},
	{"LDA $%1\n", 2},
	{"LDX $%1\n", 2},
	{"LDA [$%1]\n", 2},
	{"TAY\n", 1},
	{"LDA #$%1\n", 2},
	{"TAX\n", 1},
	{"PLB\n", 1},
	{"LDY $%1\n", 3},
	{"LDA $%1\n", 3},
	{"LDX $%1\n", 3},
	{"LDA $%1\n", 4},
	{"BCS $%1\n", 2},
	{"LDA ($%1),Y\n", 2},
	{"LDA ($%1)\n", 2},
	{"LDA ($%1,S),Y\n", 2},
	{"LDY $%1,X\n", 2},
	{"LDA $%1,X\n", 2},
	{"LDX $%1,Y\n", 2},
	{"LDA [$%1],Y\n", 2},
	{"CLV\n", 1},
	{"LDA $%1,Y\n", 3},
	{"TSX\n", 1},
	{"TYX\n", 1},
	{"LDY $%1,X\n", 3},
	{"LDA $%1,X\n", 3},
	{"LDX $%1,Y\n", 3},
	{"LDA $%1,X\n", 4},
	{"CPY #$%1\n", 2},
	{"CMP ($%1,X)\n", 2},
	{"REP #$%1\n", 2},
	{"CMP $%1,S\n", 2},
	{"CPY $%1\n", 2},
	{"CMP $%1\n", 2},
	{"DEC $%1\n", 2},
	{"CMP [$%1]\n", 2},
	{"INY\n", 1},
	{"CMP #$%1\n", 2},
	{"DEX\n", 1},
	{"WAI\n", 1},
	{"CPY $%1\n", 3},
	{"CMP $%1\n", 3},
	{"DEC $%1\n", 3},
	{"CMP $%1\n", 4},
	{"BNE $%1\n", 2},
	{"CMP ($%1),Y\n", 2},
	{"CMP ($%1)\n", 2},
	{"CMP ($%1,S),Y\n", 2},
	{"PEI ($%1)\n", 2},
	{"CMP $%1,X\n", 2},
	{"DEC $%1,X\n", 2},
	{"CMP [$%1],Y\n", 2},
	{"CLD\n", 1},
	{"CMP $%1,Y\n", 3},
	{"PHX\n", 1},
	{"STP\n", 1},
	{"JMP [$%1]\n", 3},
	{"CMP $%1,X\n", 3},
	{"DEC $%1,X\n", 3},
	{"CMP $%1,X\n", 4},
	{"CPX #$%1\n", 2},
	{"SBC ($%1,X)\n", 2},
	{"SEP #$%1\n", 2},
	{"SBC $%1,S\n", 2},
	{"CPX $%1\n", 2},
	{"SBC $%1\n", 2},
	{"INC $%1\n", 2},
	{"SBC [$%1]\n", 2},
	{"INX\n", 1},
	{"SBC #$%1\n", 2},
	{"NOP\n", 1},
	{"XBA\n", 1},
	{"CPX $%1\n", 3},
	{"SBC $%1\n", 3},
	{"INC $%1\n", 3},
	{"SBC $%1\n", 4},
	{"BEQ $%1\n", 2},
	{"SBC ($%1),Y\n", 2},
	{"SBC ($%1)\n", 2},
	{"SBC ($%1,S),Y\n", 2},
	{"PEA $%1\n", 3},
	{"SBC $%1,X\n", 2},
	{"INC $%1,X\n", 2},
	{"SBC [$%1],Y\n", 2},
	{"SED\n", 1},
	{"SBC $%1,Y\n", 3},
	{"PLX\n", 1},
	{"XCE\n", 1},
	{"JSR ($%1,X)\n", 3},
	{"SBC $%1,X\n", 3},
	{"INC $%1,X\n", 3},
	{"SBC $%1,X\n", 4}
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
