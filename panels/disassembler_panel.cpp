#include "disassembler_panel.h"
#include "debug.h"
#include "disassembly_cores/isa_65c816.h"
#include "disassembly_cores/isa_spc700.h"
#include "events/event_types.h"
#include "utility.h"

disassembler_panel::disassembler_panel(panel_manager *parent, hex_editor *editor) :
        QPlainTextEdit(parent), abstract_panel(parent, editor)
{
	cores.insert(isa_65c816::id(), new isa_65c816(this));
	cores.insert(isa_spc700::id(), new isa_spc700(this));
        for(auto i = cores.begin(); i != cores.end(); i++){
		disassembler_cores->addItem(i.key());
        }
	
	setReadOnly(true);
	connect(disassembler_cores, resolve<int>::from(&QComboBox::activated), 
	        this, &disassembler_panel::update_core_layout);
}

void disassembler_panel::disassemble(selection selection_area, const ROM_buffer *buffer)
{
	setPlainText(active_core()->disassemble(selection_area, buffer));
	state = true;
	toggle_event(DISASSEMBLER);
}

// This method makes me very sad.
void disassembler_panel::update_core_layout(int a)
{
	Q_UNUSED(a);
	QLayoutItem *child;
	while ((child = core_layout->layout()->takeAt(0))) {
		child->widget()->setParent(0);
		delete child;
	}
	delete core_layout;
	core_layout = new QWidget(this);
	box->addWidget(core_layout);
	core_layout->setLayout(active_core()->core_layout());
}

QLayout *disassembler_panel::get_layout()
{	
	box->addWidget(disassembler_cores);
	box->addWidget(this);
	box->addWidget(core_layout);
	core_layout->setLayout(active_core()->core_layout());
	return box;
}

bool disassembler_panel::state = false;
