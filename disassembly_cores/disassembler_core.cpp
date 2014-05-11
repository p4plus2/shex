#include "disassembler_core.h"
#include <QStringBuilder>
#include "debug.h"

QString disassembler_core::add_label(int destination)
{
	block &b = disassembly_list[destination];
	if(b.label.isEmpty()){
		label_id++;
		b.label = QString("label_") + QString::number(label_id);
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
			text = text % b.label % ":\n";
		}
		if(!b.mnemonic.isEmpty()){
			text = text % (label_id > 0 ? "\t" : "") % b.mnemonic % "\n";
		}
	}
	return text;
}

void disassembler_core::reset()
{
	disassembly_list.clear();
	label_id = 0;
}
