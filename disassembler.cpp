#include "disassembler.h"
#include "debug.h"
#include "disassembly_cores/isa_65c816.h"


disassembler::disassembler(QWidget *parent) :
        QPlainTextEdit(parent)
{
	cores.insert(isa_65c816::id(), new isa_65c816(this));
        for(auto i = cores.begin(); i != cores.end(); i++){
		disassembler_cores->addItem(i.key());
        }
	if(!display){
		disassembler_cores->hide();
		hide();
		core_layout->hide();
	}
	setReadOnly(true);
}

void disassembler::disassemble(selection selection_area, const ROM_buffer *buffer)
{
	setPlainText(active_core()->disassemble(selection_area, buffer));
	show();
	toggle_display(true);
}

void disassembler::toggle_display(bool state) { 
	if(state == display){
		return;
	}
	setVisible(state);
	disassembler_cores->setVisible(state);
	core_layout->setVisible(state);
	display = state;
	layout_adjust();
}

QVBoxLayout *disassembler::get_layout()
{	
	box->addWidget(disassembler_cores);
	box->addWidget(this);
	box->addWidget(core_layout);
	core_layout->setLayout(active_core()->core_layout());
	return box;
}

void disassembler::layout_adjust()
{
        QWidget *parent = parentWidget();
        while(parent){
		int height = parent->height();
		parent->setUpdatesEnabled(false);
		parent->adjustSize();
		parent->resize(parent->width(), height);
		parent->setUpdatesEnabled(true);
		parent->repaint();
		parent = parent->parentWidget();
	}
}

bool disassembler::display = false;
